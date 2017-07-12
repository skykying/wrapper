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

#include "start.h"

#include <multipass/cli/argparser.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

mp::ReturnCode cmd::Start::run(mp::ArgParser* parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [this](mp::StartReply& reply) {
        return ReturnCode::Ok;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "start failed: " << status.error_message() << "\n";
        return ReturnCode::CommandFail;
    };

    return dispatch(&RpcMethod::start, request, on_success, on_failure);
}

std::string cmd::Start::name() const { return "start"; }

QString cmd::Start::short_help() const
{
    return QStringLiteral("Start an instance");
}

QString cmd::Start::description() const
{
    return QStringLiteral("Start the named instance. Exits with return code 0\n"
                          "when the instance starts, or with an error code if\n"
                          "it fails to start.");
}

mp::ParseCode cmd::Start::parse_args(mp::ArgParser* parser)
{
    parser->addPositionalArgument("name", "Name of the instance to start", "<name>");

    auto status = parser->commandParse(this);

    if (status != ParseCode::Ok)
    {
        return status;
    }

    if (parser->positionalArguments().count() == 0)
    {
        cerr << "Name argument is required" << std::endl;
        status = ParseCode::CommandLineError;
    }
    else if (parser->positionalArguments().count() > 1)
    {
        cerr << "Too many arguments given" << std::endl;
        status = ParseCode::CommandLineError;
    }
    else
    {
        request.set_instance_name(parser->positionalArguments().first().toStdString());
    }

    return status;
}
