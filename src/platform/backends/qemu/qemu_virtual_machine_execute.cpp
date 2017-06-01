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

namespace mp = multipass;

std::string mp::QemuVirtualMachineExecute::execute()
{
    std::string full_command = "ssh -p 2222 ubuntu@localhost";

    return full_command;
}

std::string mp::QemuVirtualMachineExecute::execute(std::string command)
{
    std::string full_command = "ssh -p 2222 ubuntu@localhost";

    full_command.append(" " + command);

    return full_command;
}
