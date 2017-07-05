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

#include <multipass/platform.h>
#include <multipass/virtual_machine_factory.h>

#ifdef MULTIPASS_PLATFORM_POSIX
#include "backends/qemu/openssh_key_provider.h"
#include "backends/qemu/qemu_virtual_machine_execute.h"
#include "backends/qemu/qemu_virtual_machine_factory.h"
#else
#include "backends/hyperv/hyperv_virtual_machine_factory.h"
#endif

namespace mp = multipass;

// TODO: runtime platform checks?

std::string mp::Platform::default_server_address()
{
    return {"localhost:50051"};
}

mp::VirtualMachineFactory::UPtr mp::Platform::vm_backend()
{
#ifdef MULTIPASS_PLATFORM_POSIX
    return std::make_unique<QemuVirtualMachineFactory>();
#else
    return std::make_unique<HyperVVirtualMachineFactory>();
#endif
}

mp::VirtualMachineExecute::UPtr mp::Platform::vm_execute()
{
#ifdef MULTIPASS_PLATFORM_POSIX
    return std::make_unique<QemuVirtualMachineExecute>();
#else
    return nullptr;
#endif
}

std::unique_ptr<mp::SshPubKey> mp::Platform::public_key()
{
#ifdef MULTIPASS_PLATFORM_POSIX
    return OpenSSHKeyProvider::public_key();
#else
    return nullptr;
#endif
}
