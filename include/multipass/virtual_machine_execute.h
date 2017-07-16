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

#ifndef MULTIPASS_VIRTUAL_MACHINE_EXECUTE_H
#define MULTIPASS_VIRTUAL_MACHINE_EXECUTE_H

#include <memory>
#include <string>
#include <vector>

namespace multipass
{
class VirtualMachineExecute
{
public:
    using UPtr = std::unique_ptr<VirtualMachineExecute>;
    virtual ~VirtualMachineExecute() = default;

    virtual std::vector<std::string> execute(int port) = 0;
    virtual std::vector<std::string> execute(int port, const std::vector<std::string>& command) = 0;

protected:
    VirtualMachineExecute() = default;
    VirtualMachineExecute(const VirtualMachineExecute&) = delete;
    VirtualMachineExecute& operator=(const VirtualMachineExecute&) = delete;
};
}

#endif // MULTIPASS_VIRTUAL_MACHINE_EXECUTE_H
