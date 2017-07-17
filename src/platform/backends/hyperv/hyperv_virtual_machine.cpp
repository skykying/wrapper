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

#include "hyperv_virtual_machine.h"

namespace mp = multipass;

void mp::HyperVVirtualMachine::start()
{
}

void mp::HyperVVirtualMachine::stop()
{
}

void mp::HyperVVirtualMachine::shutdown()
{
}

mp::VirtualMachine::State mp::HyperVVirtualMachine::current_state()
{
    return mp::VirtualMachine::State::off;
}

int mp::HyperVVirtualMachine::forwarding_port()
{
    return 42;
}

void mp::HyperVVirtualMachine::wait_until_ssh_up(std::chrono::milliseconds timeout)
{
}
