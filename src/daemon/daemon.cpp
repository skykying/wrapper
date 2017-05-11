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

#include "daemon.h"
#include "daemon_config.h"

#include <multipass/virtual_machine_description.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>
#include <multipass/version.h>

#include <stdexcept>

namespace mp = multipass;

namespace
{
auto make_server(const multipass::DaemonConfig& config, multipass::Rpc::Service* service)
{
    grpc::ServerBuilder builder;

    builder.AddListeningPort(config.server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(service);

    std::unique_ptr<grpc::Server> server{builder.BuildAndStart()};
    if (server == nullptr)
        throw std::runtime_error("Failed to start the RPC service");

    return server;
}
}

mp::Daemon::Daemon(DaemonConfig config) : config{std::move(config)}, server{make_server(config, this)}
{
}

void mp::Daemon::run()
{
    std::cout << "Server listening on " << config.server_address << std::endl;
    server->Wait();
}

void mp::Daemon::shutdown() { server->Shutdown(); }

grpc::Status mp::Daemon::launch(grpc::ServerContext* context, const LaunchRequest* request,
                                LaunchReply* reply)
{
    std::cout << __func__ << std::endl;

    VirtualMachineDescription desc;
    desc.mem_size = request->mem_size();
    desc.vm_name = request->vm_name();

    vms.push_back(config.factory->create_virtual_machine(desc, *this));

    reply->set_vm_instance_name(desc.vm_name);
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::start(grpc::ServerContext* context, const StartRequest* request,
                               StartReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::stop(grpc::ServerContext* context, const StopRequest* request,
                              StopReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::destroy(grpc::ServerContext* context, const DestroyRequest* request,
                                 DestroyReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::list(grpc::ServerContext* context, const ListRequest* request,
                              ListReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::ssh(grpc::ServerContext* context, const SSHRequest* request,
                             SSHReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::version(grpc::ServerContext* context, const VersionRequest* request,
                                 VersionReply* response)
{
    response->set_version(multipass::version_string);
    return grpc::Status::OK;
}

void mp::Daemon::on_shutdown() {}

void mp::Daemon::on_resume() {}

void mp::Daemon::on_stop() {}
