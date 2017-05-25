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

#include "trash.h"

#include <multipass/cli/argparser.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

mp::ReturnCode cmd::Trash::run(ArgParser *parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [this](mp::TrashReply& reply) {
        cout << "received trash reply\n";
        return mp::ReturnCode::Ok;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "trash failed: " << status.error_message() << "\n";
        return mp::ReturnCode::CommandFail;
    };

    mp::TrashRequest request;
    return dispatch(&RpcMethod::trash, request, on_success, on_failure);
}

std::string cmd::Trash::name() const { return "trash"; }

QString cmd::Trash::short_help() const
{
    return QStringLiteral("Move an instance to trash");
}

QString cmd::Trash::description() const
{
    return QStringLiteral("The trash command moves the instance to the trash.\n");
}

mp::ParseCode cmd::Trash::parse_args(ArgParser *parser)
{
    parser->addPositionalArgument("name", "Name of instance to trash", "<name>");

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
