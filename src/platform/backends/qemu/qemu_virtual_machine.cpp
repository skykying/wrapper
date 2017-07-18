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
 * Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>
 *
 */

#include "qemu_virtual_machine.h"
#include <multipass/ssh/ssh_session.h>
#include <multipass/virtual_machine_description.h>
#include <multipass/vm_status_monitor.h>

#include <yaml-cpp/yaml.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QHostAddress>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QString>
#include <QStringList>

#include <chrono>
#include <thread>

namespace mp = multipass;

namespace
{
auto make_qemu_process(const mp::VirtualMachineDescription& desc, int ssh_port, const mp::Path& cloud_init_image)
{
    if (!QFile::exists(desc.image.image_path) || !QFile::exists(cloud_init_image))
    {
        throw std::runtime_error("cannot start VM without an image");
    }

    QStringList args{"--enable-kvm"};
    // The VM image itself
    args << "-hda" << desc.image.image_path;
    // For the cloud-init configuration
    args << "-drive" << QString{"file="} + cloud_init_image + QString{",if=virtio,format=raw"};
    // Number of cpu cores
    args << "-smp" << QString::number(desc.num_cores);
    // Memory to use for VM
    args << "-m" << QString::fromStdString(desc.mem_size);
    // Create a virtual NIC in the VM
    args << "-device"
         << "virtio-net-pci,netdev=hostnet0,id=net0";
    // Forward requested host port to guest port 22 for ssh
    args << "-netdev";
    args << QString("user,id=hostnet0,hostfwd=tcp::%1-:22").arg(ssh_port);
    // Control interface
    args << "-monitor"
         << "stdio";
    // No console
    args << "-chardev"
         // TODO Read and log machine output when verbose
         << "null,id=char0"
         << "-serial"
         << "chardev:char0"
         // TODO Add a debugging mode with access to console
         << "-nographic";

    auto process = std::make_unique<QProcess>();
    auto snap = qgetenv("SNAP");
    if (!snap.isEmpty())
    {
        process->setWorkingDirectory(snap.append("/qemu"));
    }
    qDebug() << "QProcess::workingDirectory" << process->workingDirectory();
    process->setProgram("qemu-system-x86_64");
    qDebug() << "QProcess::program" << process->program();
    process->setArguments(args);
    qDebug() << "QProcess::arguments" << process->arguments();

    return process;
}
}

mp::QemuVirtualMachine::QemuVirtualMachine(const VirtualMachineDescription& desc, const QString& cloud_init_image,
                                           int ssh_forwarding_port, VMStatusMonitor& monitor)
    : state{State::off},
      ssh_fowarding_port{ssh_forwarding_port},
      monitor{&monitor},
      vm_process{make_qemu_process(desc, ssh_forwarding_port, cloud_init_image)}
{
    QObject::connect(vm_process.get(), &QProcess::readyReadStandardOutput,
                     [this]() { qDebug("qemu.out: %s", vm_process->readAllStandardOutput().data()); });

    QObject::connect(vm_process.get(), &QProcess::readyReadStandardError,
                     [this]() { qDebug("qemu.err: %s", vm_process->readAllStandardError().data()); });

    QObject::connect(vm_process.get(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [this](int exitCode, QProcess::ExitStatus exitStatus) {
                         qDebug() << "QProcess::finished"
                                  << "exitCode" << exitCode << "exitStatus" << exitStatus;
                         on_shutdown();
                     });
}

mp::QemuVirtualMachine::~QemuVirtualMachine()
{
}

void mp::QemuVirtualMachine::start()
{
    if (state == State::running)
        return;

    vm_process->start();
    vm_process->waitForStarted();

    state = State::running;
    monitor->on_resume();
}

void mp::QemuVirtualMachine::stop()
{
    shutdown();
}

void mp::QemuVirtualMachine::shutdown()
{
    vm_process->write("system_powerdown\n");
    vm_process->waitForFinished();
}

mp::VirtualMachine::State mp::QemuVirtualMachine::current_state()
{
    return state;
}

int mp::QemuVirtualMachine::forwarding_port()
{
    return ssh_fowarding_port;
}

void mp::QemuVirtualMachine::wait_until_ssh_up(std::chrono::milliseconds timeout)
{
    using namespace std::literals::chrono_literals;

    auto deadline = std::chrono::steady_clock::now() + timeout;
    bool ssh_up{false};
    while (std::chrono::steady_clock::now() < deadline)
    {
        try
        {
            mp::SSHSession session{ssh_fowarding_port};
            ssh_up = true;
            break;
        }
        catch (const std::exception& e)
        {
            std::this_thread::sleep_for(1s);
        }
    }

    if (!ssh_up)
        throw std::runtime_error("timed out waiting for ssh service to start");
}

void mp::QemuVirtualMachine::on_shutdown()
{
    state = State::off;
    monitor->on_shutdown();
}
