/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>
 *
 */

#ifndef MULTIPASS_COMMAND_H
#define MULTIPASS_COMMAND_H

#include <grpc++/grpc++.h>
#include <rpc/multipass.grpc.pb.h>

namespace multipass
{
namespace cmd
{
class Command
{
public:
    using UPtr = std::unique_ptr<Command>;
    Command(grpc::Channel& channel, Rpc::Stub& stub, grpc::ClientContext& context)
        : rpc_channel{&channel}, stub{&stub}, context{&context}
    {
    }
    virtual ~Command() = default;
    virtual int run() = 0;
    virtual std::string name() const = 0;

protected:
    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    grpc::Channel* rpc_channel;
    Rpc::Stub* stub;
    grpc::ClientContext* context;
};
}
}
#endif // MULTIPASS_COMMAND_H
