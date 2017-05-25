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

#include <multipass/cli/argparser.h>

namespace mp = multipass;
namespace cmd = multipass::cmd;
using RpcMethod = mp::Rpc::Stub;

mp::ReturnCode cmd::Create::run(ArgParser *parser)
{
    auto ret = parse_args(parser);
    if (ret != ParseCode::Ok)
    {
        return parser->returnCodeFrom(ret);
    }

    auto on_success = [this](mp::CreateReply& reply) {
        cout << "created: " << reply.vm_instance_name() << std::endl;
        return ReturnCode::Ok;
    };

    auto on_failure = [this](grpc::Status& status) {
        cerr << "failed to create: " << status.error_message() << std::endl;
        return ReturnCode::CommandFail;
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
    request.set_mem_size(mem_size);

    return dispatch(&RpcMethod::create, request, on_success, on_failure, streaming_callback);
}

std::string cmd::Create::name() const { return "create"; }

QString cmd::Create::short_help() const
{
    return QStringLiteral("Create and start an Ubuntu instance");
}

QString cmd::Create::description() const
{
    return QStringLiteral("Create and start a new instance.");
}

mp::ParseCode cmd::Create::parse_args(ArgParser *parser)
{
    parser->addPositionalArgument("image", "Ubuntu image to start", "<image>");
    QCommandLineOption cpusOption(   {"c", "cpus"},     "Number of CPUs to allocate", "cpus", "default");
    QCommandLineOption diskOption(   {"d", "disk"},     "Disk space to allocate in bytes, or with K, M, G suffix", "disk", "default");
    QCommandLineOption memOption(    {"m", "mem"},      "Amount of memory to allocate in bytes, or with K, M, G suffix", "mem", "default");
    QCommandLineOption nameOption(   {"n", "name"},     "Name for the instance", "name");
    parser->addOptions({cpusOption, diskOption, memOption, nameOption});

    auto ret = parser->commandParse(this);

    // TODO: sanity check the command line options passed
//    if (parser->positionalArguments().count() > 1) {
//        cerr << "Too many parameters supplied" << std::endl;
//        return ReturnCode::CommandLineError;
//    }
    return ret;
}
