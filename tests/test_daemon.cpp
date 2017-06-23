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
#include <multipass/ssh_key.h>
#include <multipass/version.h>
#include <multipass/virtual_machine_execute.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>

#include "mock_virtual_machine_factory.h"
#include "mock_vm_image_fetcher.h"
#include "stub_image_host.h"
#include "stub_virtual_machine_factory.h"
#include "stub_vm_image_vault.h"

#include <gtest/gtest.h>

#include <QCoreApplication>

#include <memory>
#include <sstream>
#include <string>
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
    MOCK_METHOD3(create,
                 grpc::Status(grpc::ServerContext*, const mp::CreateRequest*, grpc::ServerWriter<mp::CreateReply>*));
    MOCK_METHOD3(destroy, grpc::Status(grpc::ServerContext*, const mp::DestroyRequest*, mp::DestroyReply*));
    MOCK_METHOD3(list, grpc::Status(grpc::ServerContext*, const mp::ListRequest*, mp::ListReply*));
    MOCK_METHOD3(start, grpc::Status(grpc::ServerContext*, const mp::StartRequest*, mp::StartReply*));
    MOCK_METHOD3(stop, grpc::Status(grpc::ServerContext*, const mp::StopRequest*, mp::StopReply*));
    MOCK_METHOD3(version, grpc::Status(grpc::ServerContext*, const mp::VersionRequest*, mp::VersionReply*));
};

struct StubNameGenerator : public mp::NameGenerator
{
    StubNameGenerator(std::string name) : name{name}
    {
    }
    std::string make_name() override
    {
        return name;
    }
    std::string name;
};
} // namespace

struct Daemon : public testing::Test
{
    void SetUp() override
    {
        loop.reset(new QEventLoop());
    }

    void send_command(std::string command, std::ostream& cout = std::cout)
    {
        send_commands({command}, cout);
    }

    void send_commands(std::vector<std::string> commands, std::ostream& cout = std::cout)
    {
        // Commands need to be sent from a thread different from that the QEventLoop is on.
        // Event loop is started/stopped to ensure all signals are delivered
        std::thread t([this, &commands, &cout]() {
            mp::ClientConfig client_config{server_address, cout, std::cerr};
            mp::Client client{client_config};
            for (const auto& command : commands)
            {
                client.run(command);
            }
            loop->quit();
        });
        loop->exec();
        t.join();
    }

#ifdef WIN32
    std::string server_address{"localhost:50051"};
#else
    std::string server_address{"unix:/tmp/test-multipassd.socket"};
#endif
    std::unique_ptr<QEventLoop> loop; // needed as cross-thread signal/slots used internally by mp::Daemon
};

TEST_F(Daemon, receives_commands)
{
    MockDaemon daemon{make_config(server_address)};

    EXPECT_CALL(daemon, connect(_, _, _));
    EXPECT_CALL(daemon, create(_, _, _));
    EXPECT_CALL(daemon, destroy(_, _, _));
    EXPECT_CALL(daemon, list(_, _, _));
    EXPECT_CALL(daemon, start(_, _, _));
    EXPECT_CALL(daemon, stop(_, _, _));
    EXPECT_CALL(daemon, version(_, _, _));

    send_commands({"connect", "create", "destroy", "list", "start", "stop", "version"});
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
    mp::Daemon daemon{config_builder.build()};

    EXPECT_CALL(*mock_factory_ptr, create_virtual_machine(_, _))
        .WillOnce(Return(ByMove(std::make_unique<StubVirtualMachine>())));

    EXPECT_CALL(*mock_factory_ptr, create_image_fetcher(_))
        .WillOnce(Return(ByMove(std::make_unique<StubVMImageFetcher>())));

    send_command("create");
}

TEST_F(Daemon, creation_calls_fetch_on_vmimagefetcher)
{
    auto mock_factory = std::make_unique<MockVirtualMachineFactory>();
    auto mock_factory_ptr = mock_factory.get();

    mp::DaemonConfigBuilder config_builder;
    config_builder.factory = std::move(mock_factory);
    config_builder.image_host = std::make_unique<StubVMImageHost>();
    config_builder.vault = std::make_unique<StubVMImageVault>();
    config_builder.server_address = server_address;
    mp::Daemon daemon{config_builder.build()};

    EXPECT_CALL(*mock_factory_ptr, create_virtual_machine(_, _))
        .WillOnce(Return(ByMove(std::make_unique<StubVirtualMachine>())));

    EXPECT_CALL(*mock_factory_ptr, create_image_fetcher(_)).WillOnce(Invoke([](auto const&) {
        auto fetcher = std::make_unique<MockVMImageFetcher>();
        EXPECT_CALL(*fetcher.get(), fetch(_));
        return std::move(fetcher);
    }));

    send_command("create");
}

TEST_F(Daemon, provides_version)
{
    mp::Daemon daemon{make_config(server_address)};

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
    config_builder.image_host = std::make_unique<StubVMImageHost>();
    mp::Daemon daemon{config_builder.build()};

    std::stringstream stream;

    send_command("create", stream);

    EXPECT_THAT(stream.str(), HasSubstr(expected_name));
}

MATCHER_P2(YAMLNodeContainsString, key, val, "")
{
    if (!arg.IsMap())
    {
        return false;
    }
    if (!arg[key])
    {
        return false;
    }
    if (!arg[key].IsScalar())
    {
        return false;
    }
    return arg[key].Scalar() == val;
}

MATCHER_P2(YAMLNodeContainsStringArray, key, values, "")
{
    if (!arg.IsMap())
    {
        return false;
    }
    if (!arg[key])
    {
        return false;
    }
    if (!arg[key].IsSequence())
    {
        return false;
    }
    if (arg[key].size() != values.size())
    {
        return false;
    }
    for (auto i = 0u; i < values.size(); ++i)
    {
        if (arg[key][i].template as<std::string>() != values[i])
        {
            return false;
        }
    }
    return true;
}

MATCHER_P(YAMLNodeContainsMap, key, "")
{
    if (!arg.IsMap())
    {
        return false;
    }
    if (!arg[key])
    {
        return false;
    }
    return arg[key].IsMap();
}

TEST_F(Daemon, default_cloud_init_grows_root_fs)
{
    auto mock_factory = std::make_unique<MockVirtualMachineFactory>();
    auto mock_factory_ptr = mock_factory.get();

    mp::DaemonConfigBuilder config_builder;
    config_builder.factory = std::move(mock_factory);
    config_builder.image_host = std::make_unique<StubVMImageHost>();
    config_builder.vault = std::make_unique<StubVMImageVault>();
    config_builder.server_address = server_address;
    mp::Daemon daemon{config_builder.build()};

    EXPECT_CALL(*mock_factory_ptr, create_virtual_machine(_, _))
        .WillOnce(Invoke([](auto const& description, auto const&) {
            YAML::Node const& cloud_init_config = description.cloud_init_config;
            EXPECT_THAT(cloud_init_config, YAMLNodeContainsMap("growpart"));

            if (cloud_init_config["growpart"])
            {
                auto const& growpart_stanza = cloud_init_config["growpart"];

                EXPECT_THAT(growpart_stanza, YAMLNodeContainsString("mode", "auto"));
                EXPECT_THAT(growpart_stanza, YAMLNodeContainsStringArray("devices", std::vector<std::string>({"/"})));
                EXPECT_THAT(growpart_stanza, YAMLNodeContainsString("ignore_growroot_disabled", "false"));
            }
            return std::make_unique<StubVirtualMachine>();
        }));

    EXPECT_CALL(*mock_factory_ptr, create_image_fetcher(_))
        .WillOnce(Return(ByMove(std::make_unique<StubVMImageFetcher>())));

    send_command("create");
}
