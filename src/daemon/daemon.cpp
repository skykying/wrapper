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
#include <multipass/query.h>
#include <multipass/ssh_key.h>
#include <multipass/version.h>
#include <multipass/virtual_machine_description.h>
#include <multipass/virtual_machine_execute.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>

#include <yaml-cpp/yaml.h>

#include <sstream>
#include <stdexcept>

namespace mp = multipass;

namespace
{
mp::Query query_from(const mp::CreateRequest* request, const std::string& name)
{
    // TODO: persistence should be specified by the rpc as well
    return {name, request->image(), false};
}

auto make_cloud_init_config(const mp::SshPubKey& key)
{
    auto config = YAML::Load(mp::base_cloud_init_config);
    std::stringstream ssh_key_line;
    ssh_key_line << "ssh-rsa"
                 << " " << key.as_base64() << " "
                 << "multipass@localhost";
    config["ssh_authorized_keys"].push_back(ssh_key_line.str());
    return config;
}

mp::VirtualMachineDescription to_machine_desc(const mp::CreateRequest* request, const std::string& name,
                                              const mp::VMImage& image, YAML::Node cloud_init_config)
{
    using mpvm = mp::VirtualMachineDescription;
    return {
        request->num_cores(),        request->mem_size(), static_cast<mpvm::MBytes>(request->disk_space()), name, image,
        std::move(cloud_init_config)};
}

template <typename T>
auto name_from(const mp::CreateRequest* request, mp::NameGenerator& name_gen, const T& currently_used_names)
{
    auto requested_name = request->instance_name();
    if (requested_name.empty())
    {
        auto name = name_gen.make_name();
        constexpr int num_retries = 100;
        for (int i = 0; i < num_retries; i++)
        {
            if (currently_used_names.find(name) != currently_used_names.end())
                continue;
            return name;
        }
        throw std::runtime_error("unable to generate a unique name");
    }
    return requested_name;
}
}

mp::DaemonRunner::DaemonRunner(const std::string& server_address, Daemon* daemon)
    : daemon_rpc{server_address}, daemon_thread{[this, daemon] {
          QObject::connect(&daemon_rpc, &DaemonRpc::on_create, daemon, &Daemon::create, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_empty_trash, daemon, &Daemon::empty_trash,
                           Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_exec, daemon, &Daemon::exec, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_info, daemon, &Daemon::info, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_list, daemon, &Daemon::list, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_recover, daemon, &Daemon::recover, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_start, daemon, &Daemon::start, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_stop, daemon, &Daemon::stop, Qt::BlockingQueuedConnection);
          QObject::connect(&daemon_rpc, &DaemonRpc::on_trash, daemon, &Daemon::trash, Qt::BlockingQueuedConnection);
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

grpc::Status mp::Daemon::create(grpc::ServerContext* context, const CreateRequest* request,
                                grpc::ServerWriter<CreateReply>* server)
try
{
    auto name = name_from(request, *config->name_generator, vm_instances);

    if (vm_instances.find(name) != vm_instances.end())
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" already exists", "");
    }

    auto query = query_from(request, name);
    auto download_monitor = [server](int percentage) {
        CreateReply create_reply;
        create_reply.set_download_progress(std::to_string(percentage));
        server->Write(create_reply);
    };

    auto prepare_action = [this](const VMImage& source_image) -> VMImage {
        return config->factory->prepare(source_image);
    };

    auto fetch_type = config->factory->fetch_type();
    auto vm_image = config->vault->fetch_image(fetch_type, query, prepare_action, download_monitor);
    auto cloud_init_config = make_cloud_init_config(*config->ssh_key);
    auto vm_desc = to_machine_desc(request, name, vm_image, std::move(cloud_init_config));

    vm_instances[name] = config->factory->create_virtual_machine(vm_desc, *this);

    CreateReply reply;
    reply.set_create_complete("Create setup complete.");
    server->Write(reply);

    reply.set_vm_instance_name(name);
    server->Write(reply);

    return grpc::Status::OK;
}
catch(const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::empty_trash(grpc::ServerContext* context, const EmptyTrashRequest* request,
                                     EmptyTrashReply* response)
{
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Command not implemented", "");
}

grpc::Status mp::Daemon::exec(grpc::ServerContext* context, const ExecRequest* request, ExecReply* response)
{
    if (request->command_line().empty())
    {
        response->set_exec_line(config->vm_execute->execute());
    }
    else
    {
        response->set_exec_line(config->vm_execute->execute(request->command_line()));
    }

    return grpc::Status::OK;
}

grpc::Status mp::Daemon::info(grpc::ServerContext* context, const InfoRequest* request, InfoReply* response)
{
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Command not implemented", "");
}

grpc::Status mp::Daemon::list(grpc::ServerContext* context, const ListRequest* request, ListReply* response)
{
    auto status_for = [](mp::VirtualMachine::State state)
    {
        switch(state)
        {
        case mp::VirtualMachine::State::running:
            return mp::ListVMInstance::RUNNING;
        default:
            return mp::ListVMInstance::STOPPED;
        }
    };

    for (const auto& instance : vm_instances)
    {
        const auto& name = instance.first;
        const auto& vm = instance.second;
        auto entry = response->add_instances();
        entry->set_name(name);
        entry->set_status(status_for(vm->current_state()));
    }

    return grpc::Status::OK;
}

grpc::Status mp::Daemon::recover(grpc::ServerContext* context, const RecoverRequest* request, RecoverReply* response)
{
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Command not implemented", "");
}

grpc::Status mp::Daemon::start(grpc::ServerContext* context, const StartRequest* request, StartReply* response)
{
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Command not implemented", "");
}

grpc::Status mp::Daemon::stop(grpc::ServerContext* context, const StopRequest* request, StopReply* response)
{
    const auto name = request->instance_name();
    auto it = vm_instances.find(name);
    if (it == vm_instances.end())
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" does not exist", "");
    }

    it->second->shutdown();
    return grpc::Status::OK;
}

grpc::Status mp::Daemon::trash(grpc::ServerContext* context, const TrashRequest* request, TrashReply* response)
{
    return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "Command not implemented", "");
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
