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

#include "qemu_virtual_machine_factory.h"
#include "qemu_virtual_machine.h"

#include <multipass/virtual_machine_description.h>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QTcpSocket>

namespace mp = multipass;

namespace
{
int find_port()
{
    QTcpSocket socket;
    if (socket.bind())
        return socket.localPort();
    throw std::runtime_error("unable to find an ssh forwarding port");
}

auto base_dir(const QString& path)
{
    QFileInfo info{path};
    return info.absoluteDir();
}

QString make_cloud_init_image(const mp::VirtualMachineDescription& desc)
{
    using namespace std::string_literals;

    const auto instance_dir = base_dir(desc.image.image_path);
    const auto cloud_init_iso = instance_dir.filePath("cloud-init-config.iso");

    if (QFile::exists(cloud_init_iso))
        return cloud_init_iso;

    QFile metadata{instance_dir.filePath("meta-data")};
    QFile userdata{instance_dir.filePath("user-data")};

    if (!metadata.open(QIODevice::WriteOnly) || !userdata.open(QIODevice::WriteOnly))
    {
        throw std::runtime_error{"failed to open files for writing during cloud-init generation"};
    }

    const auto& name = desc.vm_name;

    metadata.write("instance-id: ");
    metadata.write(name.c_str());
    metadata.write("\n");
    metadata.write("local-hostname: ");
    metadata.write(name.c_str());
    metadata.write("\n");
    metadata.close();

    YAML::Emitter userdata_emitter;

    userdata_emitter.SetIndent(4);
    userdata_emitter << desc.cloud_init_config;

    if (!userdata_emitter.good())
    {
        throw std::runtime_error{"Failed to emit cloud-init config: "s + userdata_emitter.GetLastError()};
    }

    userdata.write("#cloud-config\n");
    userdata.write(userdata_emitter.c_str());
    userdata.write("\n");
    userdata.close();


    QStringList creator_args;
    creator_args << "-o" << cloud_init_iso;
    creator_args << "-volid"
                 << "cidata"
                 << "-joliet"
                 << "-rock";
    creator_args << metadata.fileName() << userdata.fileName();

    QProcess iso_creator;
    iso_creator.start("genisoimage", creator_args);

    iso_creator.waitForFinished();

    if (iso_creator.exitCode() != QProcess::NormalExit)
    {
        throw std::runtime_error{"Call to genisoimage failed: "s + iso_creator.readAllStandardError().data()};
    }

    metadata.remove();
    userdata.remove();

    return cloud_init_iso;
}
}

mp::VirtualMachine::UPtr mp::QemuVirtualMachineFactory::create_virtual_machine(const VirtualMachineDescription& desc,
                                                                               VMStatusMonitor& monitor)
{
    return std::make_unique<mp::QemuVirtualMachine>(desc, make_cloud_init_image(desc), find_port(), monitor);
}

mp::FetchType mp::QemuVirtualMachineFactory::fetch_type()
{
    return mp::FetchType::ImageOnly;
}

mp::VMImage mp::QemuVirtualMachineFactory::prepare(const mp::VMImage& source_image)
{
    return source_image;
}
