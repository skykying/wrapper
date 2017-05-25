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

#include "stop.h"

#include <multipass/cli/argparser.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

mp::ReturnCode cmd::Stop::run(ArgParser *parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [this](mp::StopReply& reply) {
        cout << "received stop reply\n";
        return ReturnCode::Ok;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "stop failed: " << status.error_message() << "\n";
        return ReturnCode::CommandFail;
    };

    mp::StopRequest request;
    return dispatch(&RpcMethod::stop, request, on_success, on_failure);
}

std::string cmd::Stop::name() const { return "stop"; }

QString cmd::Stop::short_help() const
{
    return QStringLiteral("Stop a running instance");
}

QString cmd::Stop::description() const
{
    return QStringLiteral("Stop the named instance, if running. Exits with\n"
                          "return code 0 if successful.");
}

mp::ParseCode cmd::Stop::parse_args(ArgParser *parser)
{
    parser->addPositionalArgument("name", "Name of instance to stop", "<name>");

    auto ret = parser->commandParse(this);
    if (ret != ParseCode::Ok)
        return ret;

    if (parser->positionalArguments().count() != 1)
    {
        cerr << "Name argument is required" << std::endl;
        return ParseCode::CommandLineError;
    }
    return ret;
}
