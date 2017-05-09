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
#include <multipass/virtual_machine_description.h>
#include <multipass/vm_status_monitor.h>

#include <QProcess>
#include <QProcessEnvironment>
#include <QStringList>

namespace mp = multipass;

namespace
{
auto make_qemu_process(const mp::VirtualMachineDescription& desc)
{
    QStringList args{"--enable-kvm"};
    QProcessEnvironment env;
    env.insert("DISPLAY", ":0");

    auto process = std::make_unique<QProcess>();
    process->setProgram("/usr/bin/qemu-system-x86_64");
    process->setArguments(args);
    process->setProcessEnvironment(env);

    process->start();
    return process;
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
