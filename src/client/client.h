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

#ifndef MULTIPASS_CLIENT_H
#define MULTIPASS_CLIENT_H

#include "cmd/command.h"
#include <multipass/cli/cli.h>
#include <multipass/rpc/multipass.grpc.pb.h>

#include <memory>
#include <unordered_map>

namespace multipass
{
class Client
{
public:
    Client(std::string server_address);
    int run(const cli::Args& args);
    int run(std::string command);

private:
    void add_command(cmd::Command::UPtr);
    std::shared_ptr<grpc::Channel> rpc_channel;
    std::unique_ptr<multipass::Rpc::Stub> stub;
    grpc::ClientContext context;

    std::unordered_map<std::string, cmd::Command::UPtr> commands;
};
}
#endif // MULTIPASS_CLIENT_H
