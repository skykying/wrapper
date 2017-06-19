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
#include "qemu_vm_image_fetcher.h"

namespace mp = multipass;

mp::VirtualMachine::UPtr
mp::QemuVirtualMachineFactory::create_virtual_machine(const VirtualMachineDescription& desc,
                                                      VMStatusMonitor& monitor)
{
    return std::make_unique<mp::QemuVirtualMachine>(desc, monitor);
}

std::unique_ptr<mp::VMImageFetcher>
mp::QemuVirtualMachineFactory::create_image_fetcher(const std::unique_ptr<mp::VMImageHost>& host)
{
    return std::make_unique<mp::QemuVMImageFetcher>(host);
}
