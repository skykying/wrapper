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

#include <QFile>

namespace mp = multipass;

namespace
{
auto construct_ssh_command(int port)
{
    const auto private_key_path = mp::OpenSSHKeyProvider::private_key_path();

    if (!QFile::exists(private_key_path))
    {
        throw std::runtime_error{"Failed to find multipass SSH keys"};
    }

    // The following ssh command will undoubtedly change in The Future
    std::vector<std::string> ssh_cmd{"ssh", "-p", std::to_string(port), "-t", "-o", "StrictHostKeyChecking=no", "-o",
                                     "UserKnownHostsFile=/dev/null", "-i", private_key_path.toStdString(), "ubuntu@localhost"};

    return ssh_cmd;
}

// This is needed in order to make ssh's command parsing happy:/
auto escape_cmd_args_with_quotes(std::vector<std::string> command)
{
    std::vector<std::string> escaped_cmd_line;

    for (auto const& cmd : command)
    {
        escaped_cmd_line.push_back("'" + cmd + "'");
    }

    return escaped_cmd_line;
}
}

std::vector<std::string> mp::QemuVirtualMachineExecute::execute(int port)
{
    return construct_ssh_command(port);
}

std::vector<std::string> mp::QemuVirtualMachineExecute::execute(int port, std::vector<std::string> command)
{
    std::vector<std::string> constructed_cmd = construct_ssh_command(port);

    constructed_cmd.push_back("--");

    command = escape_cmd_args_with_quotes(command);
    constructed_cmd.insert(constructed_cmd.end(), command.begin(), command.end());

    return constructed_cmd;
}
