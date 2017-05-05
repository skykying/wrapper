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

#include "qemu_virtual_machine.h"
#include <multipass/vm_status_monitor.h>

#include <core/posix/exec.h>

#include <chrono>
#include <multipass/virtual_machine_description.h>
#include <thread>

namespace mp = multipass;

namespace
{
const std::string USER_HOME = std::string(std::getenv("HOME"));

auto make_qemu_process(const mp::VirtualMachineDescription& desc)
{
    const std::string program{"/usr/bin/qemu-system-x86_64"};
    const std::string vm_image = USER_HOME + std::string("/qemu/disk.img");
    const std::string cloudinit_img = USER_HOME + std::string("/qemu/cloudinit-seed.img");
    const std::vector<std::string> argv = {"--enable-kvm",
                                           // The VM image itself
                                           "-hda", vm_image,
                                           // For the cloud-init configuration
                                           "-hdb", cloudinit_img,
                                           // Memory to use for VM
                                           "-m", std::to_string(desc.mem_size),
                                           // Create a virtual NIC in the VM
                                           "-net", "nic",
                                           // Forward host port 2222 to guest port 22 for ssh
                                           "-net", "user,hostfwd=tcp::2222-:22"};
    std::map<std::string, std::string> env{{"DISPLAY", ":0"}};
    return core::posix::exec(program, argv, env, core::posix::StandardStream::stdout);
}
}

mp::QemuVirtualMachine::QemuVirtualMachine(const VirtualMachineDescription& desc,
                                           VMStatusMonitor& monitor)
    : state{State::running}, monitor{&monitor}, vm_process{make_qemu_process(desc)}
{
}

mp::QemuVirtualMachine::~QemuVirtualMachine() {}

void mp::QemuVirtualMachine::start()
{
    state = State::running;
    monitor->on_resume();
}

void mp::QemuVirtualMachine::stop()
{
    state = State::stopped;
    monitor->on_stop();
}

void mp::QemuVirtualMachine::shutdown()
{
    state = State::off;
    monitor->on_shutdown();
}

mp::VirtualMachine::State mp::QemuVirtualMachine::current_state() { return state; }
