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

    auto send_command(std::vector<std::string> command, std::ostream& cout = std::cout,
                      std::ostream& cerr = std::cerr)
    {
        mp::ClientConfig client_config{server_address, cout, cerr};
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

    // Use these to squelch the noise during tests
    std::stringstream stdout_stream;
    std::stringstream stderr_stream;
};

// Tests for no postional args given
TEST_F(Client, no_command_is_error)
{
    EXPECT_THAT(send_command({}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, no_command_help_ok)
{
    EXPECT_THAT(send_command({"-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// connect cli test
TEST_F(Client, connect_cmd_good_arguments)
{
    EXPECT_THAT(send_command({"connect", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, connect_cmd_help_ok)
{
    EXPECT_THAT(send_command({"connect", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// create cli tests
TEST_F(Client, create_cmd_good_arguments)
{
    EXPECT_THAT(send_command({"create", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, create_cmd_help_ok)
{
     EXPECT_THAT(send_command({"create", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, create_cmd_fails_multiple_args)
{
    EXPECT_THAT(send_command({"create", "foo", "bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, create_cmd_unknown_option_fails)
{
    EXPECT_THAT(send_command({"create", "-z", "2"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, create_cmd_name_option_ok)
{
    EXPECT_THAT(send_command({"create", "-n", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, create_cmd_name_option_fails_no_value)
{
    EXPECT_THAT(send_command({"create", "-n"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, create_cmd_memory_option_ok)
{
    EXPECT_THAT(send_command({"create", "-m", "1G"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, create_cmd_memory_option_fails_no_value)
{
    EXPECT_THAT(send_command({"create", "-m"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, create_cmd_cpu_option_ok)
{
    EXPECT_THAT(send_command({"create", "-c", "2"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, create_cmd_cpu_option_fails_no_value)
{
    EXPECT_THAT(send_command({"create", "-c"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

// empty-trash cli tests
TEST_F(Client, empty_trash_cmd_ok_no_args)
{
    EXPECT_THAT(send_command({"empty-trash"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, empty_trash_cmd_fails_with_args)
{
    EXPECT_THAT(send_command({"empty-trash", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, empty_trash_cmd_help_ok)
{
    EXPECT_THAT(send_command({"empty-trash", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// exec cli tests
TEST_F(Client, exec_cmd_double_dash_ok_cmd_arg)
{
    EXPECT_THAT(send_command({"exec", "foo", "--", "cmd"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, exec_cmd_double_dash_ok_cmd_arg_with_opts)
{
    EXPECT_THAT(send_command({"exec", "foo", "--", "cmd", "--foo", "--bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, exec_cmd_double_dash_fails_missing_cmd_arg)
{
    EXPECT_THAT(send_command({"exec", "foo", "--"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, exec_cmd_no_double_dash_ok_cmd_arg)
{
    EXPECT_THAT(send_command({"exec", "foo", "cmd"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, exec_cmd_no_double_dash_ok_multiple_args)
{
    EXPECT_THAT(send_command({"exec", "foo", "cmd", "bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, exec_cmd_no_double_dash_fails_cmd_arg_with_opts)
{
    EXPECT_THAT(send_command({"exec", "foo", "cmd", "--foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, exec_cmd_help_ok)
{
    EXPECT_THAT(send_command({"exec", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// info cli tests
TEST_F(Client, info_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"info"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, info_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"info", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, info_cmd_fails_with_multiple_args)
{
    EXPECT_THAT(send_command({"info", "foo", "bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, info_cmd_help_ok)
{
    EXPECT_THAT(send_command({"info", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// list cli tests
TEST_F(Client, list_cmd_ok_no_args)
{
    EXPECT_THAT(send_command({"list"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, list_cmd_fails_with_args)
{
    EXPECT_THAT(send_command({"list", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, list_cmd_help_ok)
{
     EXPECT_THAT(send_command({"list", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// recover cli tests
TEST_F(Client, recover_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"recover"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, recover_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"recover", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, recover_cmd_fails_with_multiple_args)
{
    EXPECT_THAT(send_command({"recover", "foo", "bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, recover_cmd_help_ok)
{
    EXPECT_THAT(send_command({"recover", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// start cli tests
TEST_F(Client, start_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"start"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, start_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"start", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, start_cmd_fails_with_multiple_args)
{
    EXPECT_THAT(send_command({"start", "foo", "bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, start_cmd_help_ok)
{
     EXPECT_THAT(send_command({"start", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// stop cli tests
TEST_F(Client, stop_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"stop"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, stop_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"stop", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, stop_cmd_fails_with_multiple_args)
{
    EXPECT_THAT(send_command({"stop", "foo", "bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, stop_cmd_help_ok)
{
     EXPECT_THAT(send_command({"stop", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

// trash cli tests
TEST_F(Client, trash_cmd_fails_no_args)
{
    EXPECT_THAT(send_command({"trash"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, trash_cmd_ok_with_one_arg)
{
    EXPECT_THAT(send_command({"trash", "foo"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, trash_cmd_fails_with_multiple_args)
{
    EXPECT_THAT(send_command({"trash", "foo", "bar"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::CommandLineError));
}

TEST_F(Client, trash_cmd_help_ok)
{
    EXPECT_THAT(send_command({"trash", "-h"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, help_returns_ok_return_code)
{
    EXPECT_THAT(send_command({"--help"}, stdout_stream, stderr_stream), Eq(mp::ReturnCode::Ok));
}

TEST_F(Client, command_help_is_different_than_general_help)
{
    std::stringstream general_help_output;
    send_command({"--help"}, general_help_output);

    std::stringstream command_output;
    send_command({"list", "--help"}, command_output);

    EXPECT_THAT(general_help_output.str(), Ne(command_output.str()));
}
