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

#include <multipass/name_generator.h>
#include <multipass/version.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>

#include "mock_virtual_machine_factory.h"
#include "stub_image_host.h"
#include "stub_virtual_machine_factory.h"
#include "stub_vm_image_vault.h"

#include <gtest/gtest.h>

#include <memory>
#include <sstream>
#include <thread>

namespace mp = multipass;
using namespace testing;

namespace
{

auto make_config(std::string server_address)
{
    mp::DaemonConfigBuilder builder;
    builder.server_address = server_address;
    return builder.build();
}

struct MockDaemon : public mp::Daemon
{
    using mp::Daemon::Daemon;
    MOCK_METHOD3(connect, grpc::Status(grpc::ServerContext*, const mp::ConnectRequest*, mp::ConnectReply*));
    MOCK_METHOD3(destroy, grpc::Status(grpc::ServerContext*, const mp::DestroyRequest*, mp::DestroyReply*));
    MOCK_METHOD3(launch, grpc::Status(grpc::ServerContext*, const mp::LaunchRequest*, mp::LaunchReply*));
    MOCK_METHOD3(list, grpc::Status(grpc::ServerContext*, const mp::ListRequest*, mp::ListReply*));
    MOCK_METHOD3(start, grpc::Status(grpc::ServerContext*, const mp::StartRequest*, mp::StartReply*));
    MOCK_METHOD3(stop, grpc::Status(grpc::ServerContext*, const mp::StopRequest*, mp::StopReply*));
    MOCK_METHOD3(version, grpc::Status(grpc::ServerContext*, const mp::VersionRequest*, mp::VersionReply*));
};

struct LaunchTrackingDaemon : public mp::Daemon
{
    using mp::Daemon::Daemon;
    grpc::Status launch(grpc::ServerContext* context, const mp::LaunchRequest* request, mp::LaunchReply* reply) override
    {
        auto status = mp::Daemon::launch(context, request, reply);
        vm_instance_name = reply->vm_instance_name();
        return status;
    }

    std::string vm_instance_name;
};

struct StubNameGenerator : public mp::NameGenerator
{
    StubNameGenerator(std::string name) : name{name} {}
    std::string make_name() override { return name; }
    std::string name;
};

template <typename DaemonType>
struct ADaemonRunner
{
    ADaemonRunner(std::unique_ptr<const mp::DaemonConfig> config) : daemon{std::move(config)}, daemon_thread{[this] { daemon.run(); }} {}

    ADaemonRunner(std::string server_address) : ADaemonRunner(make_config(server_address)) {}

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

TEST_F(Daemon, receives_commands)
{
    ADaemonRunner<MockDaemon> daemon_runner{server_address};

    EXPECT_CALL(daemon_runner.daemon, connect(_, _, _));
    EXPECT_CALL(daemon_runner.daemon, destroy(_, _, _));
    EXPECT_CALL(daemon_runner.daemon, launch(_, _, _));
    EXPECT_CALL(daemon_runner.daemon, list(_, _, _));
    EXPECT_CALL(daemon_runner.daemon, start(_, _, _));
    EXPECT_CALL(daemon_runner.daemon, stop(_, _, _));
    EXPECT_CALL(daemon_runner.daemon, version(_, _, _));

    send_commands({"connect", "destroy", "launch", "list", "start", "stop", "version"});
}

TEST_F(Daemon, creates_virtual_machines)
{
    auto mock_factory = std::make_unique<MockVirtualMachineFactory>();
    auto mock_factory_ptr = mock_factory.get();

    mp::DaemonConfigBuilder config_builder;
    config_builder.factory = std::move(mock_factory);
    config_builder.image_host = std::make_unique<StubVMImageHost>();
    config_builder.vault = std::make_unique<StubVMImageVault>();
    config_builder.server_address = server_address;
    DaemonRunner daemon_runner{config_builder.build()};

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

TEST_F(Daemon, generates_name_when_client_does_not_provide_one)
{
    const std::string expected_name{"pied-piper-valley"};

    mp::DaemonConfigBuilder config_builder;
    config_builder.server_address = server_address;
    config_builder.name_generator = std::make_unique<StubNameGenerator>(expected_name);
    config_builder.factory = std::make_unique<StubVirtualMachineFactory>();

    ADaemonRunner<LaunchTrackingDaemon> daemon_runner{config_builder.build()};

    send_command("launch");

    EXPECT_THAT(daemon_runner.daemon.vm_instance_name, Eq(expected_name));
}
