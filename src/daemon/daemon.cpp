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
#include "base_cloud_init_config.h"

#include <multipass/name_generator.h>
#include <multipass/ssh_key.h>
#include <multipass/version.h>
#include <multipass/virtual_machine_description.h>
#include <multipass/virtual_machine_execute.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image.h>
#include <multipass/vm_image_fetcher.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_query.h>
#include <multipass/vm_image_vault.h>

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <stdexcept>

namespace mp = multipass;

mp::DaemonRunner::DaemonRunner(const std::string& server_address, Daemon* daemon)
    : daemon_rpc{server_address}, daemon_thread{[this, daemon] {
          QObject::connect(&daemon_rpc, &DaemonRpc::on_create, daemon, &Daemon::create, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_connect, daemon, &Daemon::connect, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_destroy, daemon, &Daemon::destroy, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_start, daemon, &Daemon::start, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_stop, daemon, &Daemon::stop, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_list, daemon, &Daemon::list, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_version, daemon, &Daemon::version, Qt::BlockingQueuedConnection);
          daemon_rpc.run();
      }}
{
}

mp::DaemonRunner::~DaemonRunner()
{
    daemon_rpc.shutdown();
}


mp::Daemon::Daemon(std::unique_ptr<const DaemonConfig> the_config)
    : config{std::move(the_config)}, runner(config->server_address, this)
{
}

grpc::Status mp::Daemon::connect(grpc::ServerContext* context, const ConnectRequest* request, ConnectReply* response)
{
    response->set_exec_line(config->vm_execute->execute());

    return grpc::Status::OK;
}

grpc::Status mp::Daemon::create(grpc::ServerContext* context, const CreateRequest* request,
                                grpc::ServerWriter<CreateReply>* reply)
{
    VirtualMachineDescription desc;
    desc.mem_size = request->mem_size();

    if (request->vm_name().empty())
    {
        desc.vm_name = config->name_generator->make_name();
    }

    VMImageQuery vm_image_query;

    if (request->release().empty())
    {
        vm_image_query.query_string = "xenial";
    }

    std::string image_hash;

    try
    {
        config->image_host->update_image_manifest();
        image_hash = config->image_host->get_image_hash_for_query(vm_image_query.query_string);
    }
    catch (const std::runtime_error& error)
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, error.what(), "");
    }

    CreateReply create_reply;
    QObject::connect(config->image_host.get(), &mp::VMImageHost::progress, [=](int const& percentage) {
        CreateReply create_reply;
        create_reply.set_download_progress(std::to_string(percentage));
        reply->Write(create_reply);
    });

    auto fetcher = config->factory->create_image_fetcher(config->image_host);
    desc.image = fetcher->fetch(vm_image_query);

    desc.cloud_init_config = YAML::Load(mp::base_cloud_init_config);

    std::stringstream ssh_key_line;
    ssh_key_line << "ssh-rsa"
                 << " " << config->ssh_key->as_base64() << " "
                 << "multipass@localhost";

    desc.cloud_init_config["ssh_authorized_keys"].push_back(ssh_key_line.str());

    vms.push_back(config->factory->create_virtual_machine(desc, *this));

    create_reply.set_create_complete("Create setup complete.");
    reply->Write(create_reply);

    create_reply.set_vm_instance_name(desc.vm_name);
    reply->Write(create_reply);

    return grpc::Status::OK;
}

grpc::Status mp::Daemon::destroy(grpc::ServerContext* context, const DestroyRequest* request, DestroyReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::list(grpc::ServerContext* context, const ListRequest* request, ListReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::start(grpc::ServerContext* context, const StartRequest* request, StartReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::stop(grpc::ServerContext* context, const StopRequest* request, StopReply* response)
{
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::version(grpc::ServerContext* context, const VersionRequest* request, VersionReply* response)
{
    response->set_version(multipass::version_string);
    return grpc::Status::OK;
}

void mp::Daemon::on_shutdown()
{
}

void mp::Daemon::on_resume()
{
}

void mp::Daemon::on_stop()
{
}
