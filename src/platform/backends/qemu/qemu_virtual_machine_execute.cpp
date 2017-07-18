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

#include "multipass/ssh/openssh_key_provider.h"

#include <QFile>

namespace mp = multipass;

namespace
{
auto construct_ssh_command(int port, const mp::SSHKeyProvider& key_provider)
{
    // The following ssh command will undoubtedly change in The Future
    std::vector<std::string> ssh_cmd{"ssh",
                                     "-p",
                                     std::to_string(port),
                                     "-t",
                                     "-o",
                                     "StrictHostKeyChecking=no",
                                     "-o",
                                     "UserKnownHostsFile=/dev/null",
                                     "-i",
                                     key_provider.private_key_path(),
                                     "ubuntu@localhost"};

    return ssh_cmd;
}

// This is needed in order to make ssh's command parsing happy:/
auto escape_cmd_args_with_quotes(const std::vector<std::string>& command)
{
    std::vector<std::string> escaped_cmd_line;

    for (auto const& cmd : command)
    {
        escaped_cmd_line.push_back("'" + cmd + "'");
    }

    return escaped_cmd_line;
}
}
mp::QemuVirtualMachineExecute::QemuVirtualMachineExecute(const mp::SSHKeyProvider& key_provider)
    : key_provider{key_provider}
{
}

std::vector<std::string> mp::QemuVirtualMachineExecute::execute(int port)
{
    return construct_ssh_command(port, key_provider);
}

std::vector<std::string> mp::QemuVirtualMachineExecute::execute(int port, const std::vector<std::string>& command)
{
    std::vector<std::string> constructed_cmd = construct_ssh_command(port, key_provider);

    constructed_cmd.push_back("--");

    auto escaped_cmd = escape_cmd_args_with_quotes(command);
    constructed_cmd.insert(constructed_cmd.end(), escaped_cmd.begin(), escaped_cmd.end());

    return constructed_cmd;
}
