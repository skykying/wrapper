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

#include <yaml-cpp/yaml.h>

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
#include <QtCore/QTemporaryDir>
#include <QtCore/QTemporaryFile>

namespace mp = multipass;

namespace
{
std::unique_ptr<QFile> make_cloud_init_image(const YAML::Node& config, const std::string& name)
{
    using namespace std::string_literals;

    QTemporaryDir scratch_dir;

    if (!scratch_dir.isValid())
    {
        throw std::runtime_error{"Failed to create temporary directory: "s + scratch_dir.errorString().toStdString()};
    }

    QFile metadata{QDir(scratch_dir.path()).filePath("meta-data")};
    QFile userdata{QDir(scratch_dir.path()).filePath("user-data")};

    if (!metadata.open(QIODevice::WriteOnly) || !userdata.open(QIODevice::WriteOnly))
    {
        throw std::runtime_error{"Failed to open files for writing"};
    }

    metadata.write("instance-id: ");
    metadata.write(name.c_str());
    metadata.write("\n");
    metadata.write("local-hostname: ubuntu-multipass\n");

    metadata.close();

    YAML::Emitter userdata_emitter;

    userdata_emitter.SetIndent(4);
    userdata_emitter << config;

    if (!userdata_emitter.good())
    {
        throw std::runtime_error{"Failed to emit cloud-init config: "s + userdata_emitter.GetLastError()};
    }

    userdata.write("#cloud-config\n");
    userdata.write(userdata_emitter.c_str());
    userdata.write("\n");

    userdata.close();

    QProcess iso_creator;
    QStringList creator_args;

    auto cloud_init_img = std::make_unique<QTemporaryFile>();
    cloud_init_img->setFileTemplate(QDir::temp().filePath("cloud-init-config-XXXXXX.iso"));

    if (!cloud_init_img->open())
    {
        throw std::runtime_error{"Failed to create cloud-init img temporary"};
    }

    creator_args << "-o" << cloud_init_img->fileName();
    creator_args << "-volid"
                 << "cidata"
                 << "-joliet"
                 << "-rock";
    creator_args << metadata.fileName() << userdata.fileName();

    iso_creator.start("genisoimage", creator_args);

    iso_creator.waitForFinished();

    if (iso_creator.exitCode() != QProcess::NormalExit)
    {
        throw std::runtime_error{"Call to genisoimage failed: "s + iso_creator.readAllStandardError().data()};
    }

    return std::move(cloud_init_img); // explicit move to satisfy clang
}

auto make_qemu_process(const mp::VirtualMachineDescription& desc, const mp::Path& cloud_init_image)
{
    QStringList args{"--enable-kvm"};

    if (QFile::exists(desc.image.image_path) && QFile::exists(cloud_init_image))
    {
        using namespace std::string_literals;
        args <<
            // The VM image itself
            "-hda" << desc.image.image_path << // The VM image itself
             // For the cloud-init configuration
            "-drive" << QString{"file="} + cloud_init_image + QString{",if=virtio,format=raw"} <<
            // Number of cpu cores
            "-smp" << QString::number(desc.num_cores) <<
            // Memory to use for VM
            "-m" << QString::fromStdString(desc.mem_size) <<
            // Create a virtual NIC in the VM
            "-device" << "virtio-net-pci,netdev=hostnet0,id=net0" <<
            // Forward host port 2222 to guest port 22 for ssh
            "-netdev" << "user,id=hostnet0,hostfwd=tcp::2222-:22";
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

mp::QemuVirtualMachine::QemuVirtualMachine(const VirtualMachineDescription& desc, VMStatusMonitor& monitor)
    : state{State::running}, monitor{&monitor}, cloud_init_image{make_cloud_init_image(desc.cloud_init_config,
                                                                                       desc.vm_name)},
      vm_process{make_qemu_process(desc, cloud_init_image->fileName())}
{
    QObject::connect(vm_process.get(), &QProcess::readyReadStandardOutput,
                     [=]() { qDebug("qemu.out: %s", vm_process->readAllStandardOutput().data()); });

    QObject::connect(vm_process.get(), &QProcess::readyReadStandardError,
                     [=]() { qDebug("qemu.err: %s", vm_process->readAllStandardError().data()); });

    QObject::connect(vm_process.get(), static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
                     [=](int exitCode, QProcess::ExitStatus exitStatus) {
                         qDebug() << "QProcess::finished"
                                  << "exitCode" << exitCode << "exitStatus" << exitStatus;
                     });
}

mp::QemuVirtualMachine::~QemuVirtualMachine()
{
}

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

mp::VirtualMachine::State mp::QemuVirtualMachine::current_state()
{
    return state;
}
