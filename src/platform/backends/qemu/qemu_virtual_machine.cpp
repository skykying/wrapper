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
#include <multipass/virtual_machine_description.h>
#include <multipass/vm_status_monitor.h>

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QObject>
#include <QProcess>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QString>
#include <QStringList>

namespace mp = multipass;

namespace
{
auto make_qemu_process(const mp::VirtualMachineDescription& desc)
{
    const QString cloudinit_img_name("cloudinit-seed.img");
    QString cloudinit_img(QStandardPaths::locate(QStandardPaths::CacheLocation, cloudinit_img_name));

    if (!QFile::exists(cloudinit_img))
    {
        cloudinit_img = QDir(QCoreApplication::applicationDirPath()).filePath(cloudinit_img_name);
    }

    QStringList args{"--enable-kvm"};

    if (QFile::exists(desc.image_path) && QFile::exists(cloudinit_img))
    {
        using namespace std::string_literals;
        args <<  "-hda" << desc.image_path <<               // The VM image itself
                 "-drive" << ("file="s + cloudinit_img + ",if=virtio,format=raw"s).c_str() <<                 // For the cloud-init configuration
                 "-m" <<  QString::number(desc.mem_size) << // Memory to use for VM
                 "-net" << "nic" <<                         // Create a virtual NIC in the VM
                 "-net" << "user,hostfwd=tcp::2222-:22";    // Forward host port 2222 to guest port 22 for ssh
    }

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

    process->start();
    return process;
}
}

mp::QemuVirtualMachine::QemuVirtualMachine(const VirtualMachineDescription& desc,
                                           VMStatusMonitor& monitor)
    : state{State::running}, monitor{&monitor}, vm_process{make_qemu_process(desc)}
{
    QObject::connect(vm_process.get(), &QProcess::readyReadStandardOutput,
                     [=]() { qDebug("qemu.out: %s", vm_process->readAllStandardOutput().data()); });

    QObject::connect(vm_process.get(), &QProcess::readyReadStandardError,
                     [=]() { qDebug("qemu.err: %s", vm_process->readAllStandardError().data()); });

    QObject::connect(vm_process.get(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus exitStatus) {
                         qDebug() << "QProcess::finished" << "exitCode" << exitCode << "exitStatus" << exitStatus;
                     });
}

mp::QemuVirtualMachine::~QemuVirtualMachine() {}

void mp::QemuVirtualMachine::start()
{
    state = State::running;
    monitor->on_resume();
}

void mp::QemuVirtualMachine::stop()
{
    state = State::stopped;
    monitor->on_stop();
}

void mp::QemuVirtualMachine::shutdown()
{
    state = State::off;
    monitor->on_shutdown();
}

mp::VirtualMachine::State mp::QemuVirtualMachine::current_state() { return state; }
