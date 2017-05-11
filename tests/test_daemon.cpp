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

#include <multipass/version.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>

#include "mock_virtual_machine_factory.h"
#include "stub_image_host.h"
#include "stub_virtual_machine_factory.h"
#include "stub_vm_image_vault.h"

#include <gtest/gtest.h>

#include <sstream>
#include <thread>

namespace mp = multipass;
using namespace testing;

namespace
{
template <typename DaemonType>
struct ADaemonRunner
{
    ADaemonRunner(mp::DaemonConfig config) : daemon{std::move(config)}, daemon_thread{[this] { daemon.run(); }} {}

    ADaemonRunner(std::string server_address) : ADaemonRunner(mp::DaemonConfig{server_address}) {}

    ~ADaemonRunner()
    {
        daemon.shutdown();
        if (daemon_thread.joinable())
            daemon_thread.join();
    }

    DaemonType daemon;
    std::thread daemon_thread;
};

using DaemonRunner = ADaemonRunner<mp::Daemon>;
}

struct Daemon : public testing::Test
{
    void send_command(std::string command, std::ostream& cout = std::cout) { send_commands({command}, cout); }

    void send_commands(std::vector<std::string> commands, std::ostream& cout = std::cout)
    {
        mp::ClientConfig client_config{server_address, cout, std::cerr};
        mp::Client client{client_config};
        for (const auto& command : commands)
        {
            client.run(command);
        }
    }

    std::string server_address{"unix:/tmp/test-multipassd.socket"};
};

TEST_F(Daemon, creates_virtual_machines)
{
    auto mock_factory = std::make_unique<MockVirtualMachineFactory>();
    auto mock_factory_ptr = mock_factory.get();

    mp::DaemonConfig config{std::move(mock_factory), std::make_unique<StubVMImageHost>(),
                            std::make_unique<StubVMImageVault>(), server_address};

    DaemonRunner daemon_runner{std::move(config)};

    EXPECT_CALL(*mock_factory_ptr, create_virtual_machine(_, _))
        .WillOnce(Return(ByMove(std::make_unique<StubVirtualMachine>())));

    send_command("launch");
}

TEST_F(Daemon, provides_version)
{
    DaemonRunner daemon_runner{server_address};

    std::stringstream stream;
    send_command("version", stream);

    EXPECT_THAT(stream.str(), HasSubstr(mp::version_string));
}
