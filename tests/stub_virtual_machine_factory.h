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

#ifndef MULTIPASS_STUB_VIRTUAL_MACHINE_FACTORY_H
#define MULTIPASS_STUB_VIRTUAL_MACHINE_FACTORY_H

#include "stub_virtual_machine.h"
#include "stub_vm_image_fetcher.h"

#include <multipass/virtual_machine_factory.h>

struct StubVirtualMachineFactory final : public multipass::VirtualMachineFactory
{
    multipass::VirtualMachine::UPtr
    create_virtual_machine(const multipass::VirtualMachineDescription&,
                           multipass::VMStatusMonitor&) override
    {
        return std::make_unique<StubVirtualMachine>();
    }

    std::unique_ptr<multipass::VMImageFetcher>
    create_image_fetcher(const std::unique_ptr<multipass::VMImageHost>&) override
    {
        return std::make_unique<StubVMImageFetcher>();
    }
};

#endif // MULTIPASS_STUB_VIRTUAL_MACHINE_FACTORY_H
