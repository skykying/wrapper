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

#include "recover.h"

#include <multipass/cli/argparser.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

mp::ReturnCode cmd::Recover::run(mp::ArgParser* parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [this](mp::RecoverReply& reply) {
        return mp::ReturnCode::Ok;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "recover failed: " << status.error_message() << "\n";
        return mp::ReturnCode::CommandFail;
    };

    return dispatch(&RpcMethod::recover, request, on_success, on_failure);
}

std::string cmd::Recover::name() const { return "recover"; }

QString cmd::Recover::short_help() const
{
    return QStringLiteral("Recovers an instance from the trash");
}

QString cmd::Recover::description() const
{
    return QStringLiteral("Recovers the instance from the trash so it can be used again.");
}

mp::ParseCode cmd::Recover::parse_args(mp::ArgParser* parser)
{
    parser->addPositionalArgument("name", "Name of instance to recover", "<name>");

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
