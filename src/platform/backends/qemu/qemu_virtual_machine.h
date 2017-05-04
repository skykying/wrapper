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

#ifndef MULTIPASS_QEMU_VIRTUAL_MACHINE_H
#define MULTIPASS_QEMU_VIRTUAL_MACHINE_H

#include <multipass/virtual_machine.h>

#include <core/posix/child_process.h>

namespace multipass
{
class VMStatusMonitor;
struct VirtualMachineDescription;

class QemuVirtualMachine final : public VirtualMachine
{
public:
    QemuVirtualMachine(const VirtualMachineDescription& desc, VMStatusMonitor& monitor);
    ~QemuVirtualMachine();

    void start() override;
    void stop() override;
    void shutdown() override;
    State current_state() override;

private:
    VirtualMachine::State state;
    VMStatusMonitor* monitor;
    core::posix::ChildProcess vm_process;
};
}

#endif // MULTIPASS_QEMU_VIRTUAL_MACHINE_H
