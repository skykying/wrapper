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
#include <QTcpSocket>

namespace mp = multipass;

namespace
{
int find_port()
{
    QTcpSocket socket;
    if (socket.bind())
    {
        auto port = socket.localPort();
        qDebug() << "port:" << port;
        return port;
    }
    throw std::runtime_error("unable to find an ssh forwarding port");
}

}

mp::VirtualMachine::UPtr mp::QemuVirtualMachineFactory::create_virtual_machine(const VirtualMachineDescription& desc,
                                                                               VMStatusMonitor& monitor)
{
    return std::make_unique<mp::QemuVirtualMachine>(desc, find_port(), monitor);
}

mp::FetchType mp::QemuVirtualMachineFactory::fetch_type()
{
    return mp::FetchType::ImageOnly;
}

mp::VMImage mp::QemuVirtualMachineFactory::prepare(const mp::VMImage& source_image)
{
    return source_image;
}
