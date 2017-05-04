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

#include <src/client/client.h>
#include <src/daemon/daemon.h>
#include <src/daemon/daemon_config.h>

#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>

#include "mock_virtual_machine_factory.h"
#include "stub_image_host.h"
#include "stub_virtual_machine_factory.h"
#include "stub_vm_image_vault.h"

#include <gtest/gtest.h>

#include <thread>

using namespace testing;

struct Daemon : public testing::Test
{
    std::string server_address{"unix:/tmp/test-multipassd.socket"};
};

TEST_F(Daemon, creates_virtual_machines)
{
    auto mock_factory = std::make_unique<MockVirtualMachineFactory>();
    auto mock_factory_ptr = mock_factory.get();

    multipass::DaemonConfig config{std::move(mock_factory), std::make_unique<StubVMImageHost>(),
                                   std::make_unique<StubVMImageVault>(), server_address};

    multipass::Daemon daemon(config);

    std::thread t{[&daemon] { daemon.run(); }};

    EXPECT_CALL(*mock_factory_ptr, create_virtual_machine(_, _))
        .WillOnce(Return(ByMove(std::make_unique<StubVirtualMachine>())));

    multipass::Client client{server_address};
    client.run("launch");

    daemon.shutdown();
    t.join();
}
