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

#include "create.h"

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

int cmd::Create::run()
{
    auto on_success = [this](mp::CreateReply& reply) {

        cout << "created: " << reply.vm_instance_name();
        cout << std::endl;
        return EXIT_SUCCESS;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "failed to create: " << status.error_message() << std::endl;
        return EXIT_FAILURE;
    };

    auto streaming_callback = [this](mp::CreateReply& reply) {
        if (reply.create_oneof_case() == 2)
        { 
            cout << "Downloaded " << reply.download_progress() << "%" << '\r' << std::flush;
        }
        else if (reply.create_oneof_case() == 3)
        {
            cout << "\n" << reply.create_complete() << std::endl;
        }
    };

    mp::CreateRequest request;

    // Set some defaults
    request.set_mem_size(1024);

    return dispatch(&RpcMethod::create, request, on_success, on_failure, streaming_callback);
}

std::string cmd::Create::name() const { return "create"; }
