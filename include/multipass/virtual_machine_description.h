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

#ifndef MULTIPASS_VIRTUAL_MACHINE_DESCRIPTION_H
#define MULTIPASS_VIRTUAL_MACHINE_DESCRIPTION_H

#include <string>

namespace multipass
{
class VirtualMachineDescription
{
public:
    using MBytes = size_t;

    int num_cores;
    MBytes mem_size;
    MBytes disk_space;
    std::string vm_name;
};
}
#endif // MULTIPASS_VIRTUAL_MACHINE_DESCRIPTION_H
