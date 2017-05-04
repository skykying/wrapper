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

mp::Client::Client(std::string server_address)
    : rpc_channel{grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials())},
      stub{mp::Rpc::NewStub(rpc_channel)}
{
    add_command(std::make_unique<cmd::Launch>(*rpc_channel, *stub, context));
}

void mp::Client::add_command(cmd::Command::UPtr cmd) { commands[cmd->name()] = std::move(cmd); }

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