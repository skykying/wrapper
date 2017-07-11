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

#include <QDir>
#include <QFile>
#include <QFileInfo>

namespace mp = multipass;

namespace
{
constexpr int first_ephemeral_port{49152};
constexpr int last_ephemeral_port{65535};
}

mp::QemuVirtualMachineFactory::QemuVirtualMachineFactory() : next_port{first_ephemeral_port}
{
}

mp::VirtualMachine::UPtr mp::QemuVirtualMachineFactory::create_virtual_machine(const VirtualMachineDescription& desc,
                                                                               VMStatusMonitor& monitor)
{
    // TODO: check if port is actually available
    const auto port_to_use = next_port;
    if (next_port > last_ephemeral_port)
        throw std::runtime_error("No more ssh forwarding ports available");
    ++next_port;
    return std::make_unique<mp::QemuVirtualMachine>(desc, port_to_use, monitor);
}

mp::FetchType mp::QemuVirtualMachineFactory::fetch_type()
{
    return mp::FetchType::ImageOnly;
}

mp::VMImage mp::QemuVirtualMachineFactory::prepare(const mp::VMImage& source_image)
{
    return source_image;
}
