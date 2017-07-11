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
 * Authored by: Chris Townsend <christopher.townsend@canonical.com>
 *
 */

#include "info.h"

#include <multipass/cli/argparser.h>

#include <iomanip>
#include <sstream>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

namespace
{
std::ostream& operator<<(std::ostream& out, const multipass::InfoReply_Status& status)
{
    switch(status)
    {
    case mp::InfoReply::RUNNING:
        out << "RUNNING";
        break;
    case mp::InfoReply::STOPPED:
        out << "STOPPED";
        break;
    case mp::InfoReply::TRASHED:
        out << "IN TRASH";
        break;
    default:
        out << "UNKOWN";
        break;
    }
    return out;
}
}

mp::ReturnCode cmd::Info::run(mp::ArgParser* parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [this](mp::InfoReply& reply) {
        std::stringstream out;
        out << std::setw(16) << std::left << "Name:";
        out << reply.name() << "\n";

        out << std::setw(16) << std::left << "State:";
        out << reply.status() << "\n";

        out << std::setw(16) << std::left << "IPv4:";
        out << reply.ipv4() << "\n";

        out << std::setw(16) << std::left << "IPV6:";
        out << reply.ipv6() << "\n";

        out << std::setw(16) << std::left << "Ubuntu release:";
        out << reply.release() << "\n";

        out << std::setw(16) << std::left << "Image hash:";
        out << reply.id() << "\n";

        out << std::setw(16) << std::left << "Load:";
        out << reply.load() << "\n";

        out << std::setw(16) << std::left << "Disk usage:";
        out << reply.disk_usage() << "%\n";

        out << std::setw(16) << std::left << "Memory usage:";
        out << reply.memory_usage() << "%\n";

        cout << out.str();
        return ReturnCode::Ok;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "info failed: " << status.error_message() << "\n";
        return ReturnCode::CommandFail;
    };

    return dispatch(&RpcMethod::info, request, on_success, on_failure);
}

std::string cmd::Info::name() const { return "info"; }

QString cmd::Info::short_help() const
{
    return QStringLiteral("Display information about an instance");
}

QString cmd::Info::description() const
{
    return QStringLiteral("Display information about an instance");
}

mp::ParseCode cmd::Info::parse_args(mp::ArgParser* parser)
{
    parser->addPositionalArgument("name", "Name of instance to display information about", "<name>");

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
