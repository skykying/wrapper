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
 * Authored by: Chris Townsend <christopher.townsend@canonical.com>
 *
 */

#include <src/client/client.h>
#include <src/daemon/auto_join_thread.h>
#include <src/daemon/daemon_rpc.h>

#include <QEventLoop>
#include <QStringList>

#include <gmock/gmock.h>

namespace mp = multipass;
using namespace testing;

class Client : public Test
{
public:
    Client() : daemon_thread{[this] { stub_daemon.run(); }}
    {
    }

    ~Client()
    {
        stub_daemon.shutdown();
    }

    auto send_command(std::vector<std::string> command, std::ostream& cout = std::cout)
    {
        mp::ClientConfig client_config{server_address, cout, std::cerr};
        mp::Client client{client_config};
        QStringList args = QStringList() << "multipass_test";

        for (const auto& arg : command)
        {
            args << QString::fromStdString(arg);
        }
        return client.run(args);
    }

#ifdef WIN32
    std::string server_address{"localhost:50051"};
#else
    std::string server_address{"unix:/tmp/test-multipassd.socket"};
#endif
    mp::DaemonRpc stub_daemon{server_address};
    mp::AutoJoinThread daemon_thread;
};

TEST_F(Client, no_command_is_error)
{
    EXPECT_THAT(send_command({}), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, no_command_help_ok)
{
    EXPECT_THAT(send_command({"-h"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, create_cmd_good_arguments)
{
    EXPECT_THAT(send_command({"create", "foo"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, create_cmd_help_ok)
{
     EXPECT_THAT(send_command({"create", "-h"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, connect_cmd_good_arguments)
{
    EXPECT_THAT(send_command({"connect", "foo"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, connect_cmd_help_ok)
{
     EXPECT_THAT(send_command({"connect", "-h"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, list_cmd_ok_no_args)
{
    EXPECT_THAT(send_command({"list"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, list_cmd_fails_with_args)
{
    EXPECT_THAT(send_command({"list", "foo"}), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, list_cmd_help_ok)
{
     EXPECT_THAT(send_command({"list", "-h"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, start_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"start"}), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, start_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"start", "foo"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, start_cmd_help_ok)
{
     EXPECT_THAT(send_command({"start", "-h"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, stop_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"stop"}), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, stop_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"stop", "foo"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, stop_cmd_help_ok)
{
     EXPECT_THAT(send_command({"stop", "-h"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, trash_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"trash"}), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, trash_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"trash", "foo"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, trash_cmd_help_ok)
{
    EXPECT_THAT(send_command({"trash", "-h"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, help_returns_ok_return_code)
{
    EXPECT_THAT(send_command({"--help"}), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, command_help_is_different_than_general_help)
{
    std::stringstream general_help_output;
    send_command({"--help"}, general_help_output);

    std::stringstream command_output;
    send_command({"list", "--help"}, command_output);

    EXPECT_THAT(general_help_output.str(), Ne(command_output.str()));
}
