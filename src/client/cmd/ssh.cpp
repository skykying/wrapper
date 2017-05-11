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

#include "ssh.h"

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

int cmd::SSH::run()
{
    auto on_success = [this](mp::SSHReply& reply) {
        cout << "received ssh reply\n";
        return EXIT_SUCCESS;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "ssh failed: " << status.error_message() << "\n";
        return EXIT_FAILURE;
    };

    mp::SSHRequest request;
    return dispatch(&RpcMethod::ssh, request, on_success, on_failure);
}

std::string cmd::SSH::name() const { return "ssh"; }
