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

#include "version.h"
#include <multipass/version.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

int cmd::Version::run()
{
    std::cout << "ubuntu     " << multipass::version_string << std::endl;

    auto on_success = [](mp::VersionReply& reply) {
        std::cout << "multipassd " << reply.version();
        std::cout << std::endl;
        return EXIT_SUCCESS;
    };

    auto on_failure = [](grpc::Status& status) { return EXIT_SUCCESS; };

    mp::VersionRequest request;
    return dispatch(&RpcMethod::version, request, on_success, on_failure);
}

std::string cmd::Version::name() const { return "version"; }