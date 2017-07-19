/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Gerry Boland <gerry.boland@canonical.com>
 */

#include "hyperkit_virtual_machine.h"
#include <multipass/virtual_machine_description.h>
#include <multipass/vm_status_monitor.h>

#include <yaml-cpp/yaml.h>

#include <QCoreApplication>
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QString>
#include <QStringList>

#include <signal.h>

namespace mp = multipass;

namespace
{

QByteArray generateUuid(const QByteArray& string)
{
    QByteArray uuid = QCryptographicHash::hash(string, QCryptographicHash::Md5);

    // Force bits to ensure it meets RFC-4122 v3 (MD5) standard...
    uuid[6] = (uuid[6] & 0x0f) | (3 << 4); // version '3' shifted left into top nibble
    uuid[8] = (uuid[8] & 0x3f) | 0x80;

    return uuid.toHex().insert(8, '-').insert(13, '-').insert(18, '-').insert(23, '-');
}

auto make_hyperkit_process(const mp::VirtualMachineDescription& desc, const mp::Path& cloud_init_image)
{
    QStringList args{};

    if (QFile::exists(desc.image.image_path) && QFile::exists(cloud_init_image))
    {
        using namespace std::string_literals;
        args << "-q"
             << "/dev/null" // do not write the log to file
             << QCoreApplication::applicationDirPath() + "/hyperkit" <<
            // Number of cpu cores
            "-c" << QString::number(desc.num_cores) <<
            // Memory to use for VM
            "-m" << QString::fromStdString(desc.mem_size) <<
            // RTC keeps UTC
            "-u" <<
            // ACPI tables
            "-A" <<
            // Send shutdown signal to VM on SIGTERM to hyperkit
            "-H" <<
            // VM having consistent UUID ensures it gets same IP address across reboots
            "-U" << generateUuid(QByteArray::fromStdString(desc.vm_name)) <<

            // PCI devices:
            // PCI host bridge
            "-s"
             << "0:0,hostbridge" <<
            // Network (root-only)
            "-s"
             << "2:0,virtio-net" <<
            // Entropy device emulation.
            "-s"
             << "5,virtio-rnd" <<
            // LPC = low-pin-count device, used for serial console
            "-s"
             << "31,lpc" <<
            // Forward all console output to stdio, and a ring-log file
            // Note: can enable add a fixed-size circular log file with "log=/tmp/hyperkit.log" (mmap-ed)
            "-l"
             << "com1,stdio" <<
            // The VM image itself
            "-s"
             << QString{"1:0,ahci-hd,file://"} + desc.image.image_path +
                    QString{"?sync=os&buffered=1,format=qcow,qcow-config=discard=true;compact_after_unmaps=0;keep_"
                            "erased=0;runtime_asserts=false"}
             <<
            // Disk image for the cloud-init configuration
            "-s" << QString{"1:1,ahci-cd,"} + cloud_init_image
             << "-f"
             // Firmware argument
             << QString{"kexec,"} + desc.image.kernel_path + QString{","} + desc.image.initrd_path +
                    QString{",earlyprintk=serial console=ttyS0 root=/dev/sda1 rw"};
    }

    auto process = std::make_unique<QProcess>();
    auto snap = qgetenv("SNAP");
    if (!snap.isEmpty())
    {
        process->setWorkingDirectory(snap.append("/hyperkit"));
    }
    qDebug() << "QProcess::workingDirectory" << process->workingDirectory();
    process->setProgram("script"); // using "script" as hykerkit writes directly to the tty
    qDebug() << "QProcess::program" << process->program();
    process->setArguments(args);
    qDebug() << "QProcess::arguments" << process->arguments();

    return process;
}
} // namespace

mp::HyperkitVirtualMachine::HyperkitVirtualMachine(const VirtualMachineDescription& desc,
                                                   const QString& cloud_init_image, VMStatusMonitor& monitor)
    : state{State::off}, monitor{&monitor}, vm_process{make_hyperkit_process(desc, cloud_init_image)}
{
    QObject::connect(vm_process.get(), &QProcess::started, [=]() { on_start(); });
    QObject::connect(vm_process.get(), &QProcess::readyRead, [=]() {
        if (!vm_process->canReadLine()) // buffer until line available
            return;

        printf("%s", vm_process->readAllStandardOutput().constData()); // pipe VM output to stdout
    });

    QObject::connect(vm_process.get(), &QProcess::readyReadStandardError,
                     [=]() { qDebug("hyperkit.err: %s", vm_process->readAllStandardError().data()); });

    QObject::connect(vm_process.get(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus exitStatus) {
                         if (!vm_process->atEnd())
                         {
                             printf("%s", vm_process->readAllStandardError().constData());
                         }
                         qDebug() << "QProcess::finished"
                                  << "exitCode" << exitCode << "exitStatus" << exitStatus;
                         on_shutdown();
                     });
}

mp::HyperkitVirtualMachine::~HyperkitVirtualMachine()
{
    if (state == State::running)
        shutdown();
}

void mp::HyperkitVirtualMachine::start()
{
    if (state != State::running)
        vm_process->start();
}

void mp::HyperkitVirtualMachine::stop()
{
    shutdown();
}

void mp::HyperkitVirtualMachine::shutdown()
{
    if (state == State::running && vm_process->processId() > 0)
    {
        // need to get PID of the hyperkit by getting the child process of "script"
        QProcess getHyperkitPid;
        QStringList args{"-P", QString::number(vm_process->processId())};
        getHyperkitPid.start("pgrep", args);
        getHyperkitPid.waitForFinished(100);
        bool ok;
        int pid = getHyperkitPid.readAllStandardOutput().trimmed().toInt(&ok);
        if (ok && pid > 0)
        {
            // hyperkit intercepts this and sends shutdown signal to the VM
            qDebug("Sending SIGTERM to Hyperkit PID: %d", pid);
            kill(pid, SIGTERM);
        }
        else
        {
            throw std::runtime_error("unable to identify hyperkit's PID, failed to shutdown VM");
        }
        vm_process->waitForFinished();
    }
}

void mp::HyperkitVirtualMachine::on_start()
{
    state = State::running;
    monitor->on_resume();
}

void mp::HyperkitVirtualMachine::on_shutdown()
{
    state = State::off;
    monitor->on_shutdown();
}

mp::VirtualMachine::State mp::HyperkitVirtualMachine::current_state()
{
    return state;
}

int mp::HyperkitVirtualMachine::forwarding_port()
{
    return 22;
}

void mp::HyperkitVirtualMachine::wait_until_ssh_up(std::chrono::milliseconds timeout)
{
    // TODO
    return;
}
