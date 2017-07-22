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

#include <multipass/cloud_init_iso.h>
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

    const auto& name = desc.vm_name;

    std::stringstream meta_data;

    meta_data << "instance-id: " << name << "\n"
              << "local-hostname: " << name << "\n";

    YAML::Emitter userdata_emitter;

    userdata_emitter.SetIndent(4);
    userdata_emitter << desc.cloud_init_config;

    if (!userdata_emitter.good())
    {
        throw std::runtime_error{"Failed to emit cloud-init config: "s + userdata_emitter.GetLastError()};
    }

    std::stringstream user_data;
    user_data << "#cloud-config\n" << userdata_emitter.c_str() << "\n";

    mp::CloudInitIso::write_to(cloud_init_iso, meta_data.str(), user_data.str());

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
