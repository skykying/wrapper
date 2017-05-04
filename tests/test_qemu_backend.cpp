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

#include <multipass/virtual_machine.h>
#include <multipass/virtual_machine_description.h>
#include <src/platform/backends/qemu/qemu_virtual_machine_factory.h>

#include "mock_status_monitor.h"
#include "stub_status_monitor.h"

#include <gmock/gmock.h>

namespace mp = multipass;

using namespace testing;

struct QemuBackend : public testing::Test
{
    mp::VirtualMachineDescription default_description{2, 1024 * 1024 * 1024};
    mp::QemuVirtualMachineFactory backend;
};

TEST_F(QemuBackend, creates_in_running_state)
{
    StubVMStatusMonitor stub_monitor;
    auto machine = backend.create_virtual_machine(default_description, stub_monitor);
    EXPECT_THAT(machine->current_state(), Eq(mp::VirtualMachine::State::running));
}

TEST_F(QemuBackend, machine_sends_monitoring_events)
{
    mp::QemuVirtualMachineFactory backend;
    MockVMStatusMonitor mock_monitor;

    auto machine = backend.create_virtual_machine(default_description, mock_monitor);

    EXPECT_CALL(mock_monitor, on_stop());
    machine->stop();

    EXPECT_CALL(mock_monitor, on_resume());
    machine->start();

    EXPECT_CALL(mock_monitor, on_shutdown());
    machine->shutdown();
}
