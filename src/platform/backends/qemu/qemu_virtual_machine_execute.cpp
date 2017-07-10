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
 * Authored by: Chris Townsend <christopher.townsend@canonical.com>
 *
 */

#include "qemu_virtual_machine_execute.h"
#include "openssh_key_provider.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>

namespace mp = multipass;

namespace
{
auto construct_ssh_command()
{
    const auto private_key_path = mp::OpenSSHKeyProvider::private_key_path();

    if (!QFile::exists(private_key_path))
    {
        throw std::runtime_error{"Failed to find multipass SSH keys"};
    }

    // The following ssh command will undoubtedly change in The Future
    QString ssh_cmd("ssh -p 2222 -t -o StrictHostKeyChecking=no -o UserKnownHostsFile=/dev/null -i " + private_key_path + " ubuntu@localhost");

    return ssh_cmd.toStdString();
}
}

std::string mp::QemuVirtualMachineExecute::execute()
{
    return construct_ssh_command();
}

std::string mp::QemuVirtualMachineExecute::execute(std::string command)
{
    return construct_ssh_command() + " " + command;
}
