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

#include "launch.h"

namespace mp = multipass;
namespace cmd = multipass::cmd;

int cmd::Launch::run()
{
    mp::LaunchRequest request;
    mp::LaunchReply reply;

    auto status = stub->launch(context, request, &reply);

    // Act upon its status.
    if (status.ok())
    {
        std::cout << "launched: " << reply.vm_instance_name();
        std::cout << std::endl;
        return EXIT_SUCCESS;
    }

    std::cerr << "failed to launch: " << status.error_message() << std::endl;
    return EXIT_FAILURE;
}

std::string cmd::Launch::name() const { return "launch"; }