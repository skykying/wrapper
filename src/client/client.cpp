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
#include "client.h"
#include "cmd/launch.h"
#include "cmd/version.h"
#include "cmd/start.h"
#include "cmd/stop.h"
#include "cmd/list.h"
#include "cmd/ssh.h"
#include "cmd/destroy.h"

#include <grpc++/grpc++.h>

#include <algorithm>

namespace mp = multipass;

namespace
{
template <typename T>
auto sorted_cmd_names_from(const T& list)
{
    std::vector<std::string> cmd_names;
    for (const auto& kv : list)
    {
        cmd_names.push_back(kv.first);
    }
    std::sort(cmd_names.begin(), cmd_names.end());
    return cmd_names;
}
}

mp::Client::Client(const ClientConfig& config)
    : rpc_channel{grpc::CreateChannel(config.server_address, grpc::InsecureChannelCredentials())},
      stub{mp::Rpc::NewStub(rpc_channel)}, cout{config.cout}, cerr{config.cerr}
{
    add_command<cmd::Launch>();
    add_command<cmd::Version>();
    add_command<cmd::Start>();
    add_command<cmd::Stop>();
    add_command<cmd::List>();
    add_command<cmd::SSH>();
    add_command<cmd::Destroy>();
}

template <typename T>
void mp::Client::add_command()
{
    auto cmd = std::make_unique<T>(*rpc_channel, *stub, cout, cerr);
    commands[cmd->name()] = std::move(cmd);
}

int mp::Client::run(const mp::cli::Args& args)
{
    // TODO: actually parse args
    if (args.empty())
    {
        std::cout << "Usage:\n    ubuntu <command>\n\n";
        std::cout << "Available commands:\n";
        auto names = sorted_cmd_names_from(commands);
        for (auto const& name : names)
        {
            std::cout << "  ";
            std::cout << name << "\n";
        }

        return EXIT_FAILURE;
    }

    auto const cmd_name = args[0];
    return run(cmd_name);
}

int mp::Client::run(std::string cmd_name)
{
    auto& cmd = commands[cmd_name];
    if (cmd != nullptr)
        return cmd->run();

    return EXIT_FAILURE;
}