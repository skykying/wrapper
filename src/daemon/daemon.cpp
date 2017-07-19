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
#include "json_writer.h"

#include <multipass/name_generator.h>
#include <multipass/query.h>
#include <multipass/ssh/ssh_session.h>
#include <multipass/version.h>
#include <multipass/virtual_machine_description.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>

#include <yaml-cpp/yaml.h>

#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>

#include <sstream>
#include <stdexcept>

namespace mp = multipass;

namespace
{
constexpr char instance_db_name[] = "multipassd-vm-instances.json";

mp::Query query_from(const mp::CreateRequest* request, const std::string& name)
{
    // TODO: persistence should be specified by the rpc as well
    return {name, request->image(), false};
}

auto make_cloud_init_config(const mp::SSHKeyProvider& key_provider)
{
    auto config = YAML::Load(mp::base_cloud_init_config);
    std::stringstream ssh_key_line;
    ssh_key_line << "ssh-rsa"
                 << " " << key_provider.public_key_as_base64() << " "
                 << "multipass@localhost";
    config["ssh_authorized_keys"].push_back(ssh_key_line.str());
    return config;
}

mp::VirtualMachineDescription to_machine_desc(const mp::CreateRequest* request, const std::string& name,
                                              const mp::VMImage& image, YAML::Node cloud_init_config)
{
    using mpvm = mp::VirtualMachineDescription;
    auto num_cores = request->num_cores() < 1 ? 1 : request->num_cores();
    auto mem_size = request->mem_size().empty() ? "1G" : request->mem_size();
    auto disk_size = request->disk_space() <= 0 ? 0u : static_cast<mpvm::MBytes>(request->disk_space());
    return {num_cores, mem_size, disk_size, name, image, std::move(cloud_init_config)};
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

std::unordered_map<std::string, mp::VMSpecs> load_db(const mp::Path& cache_path)
{
    QDir cache_dir{cache_path};
    QFile db_file{cache_dir.filePath(instance_db_name)};
    auto opened = db_file.open(QIODevice::ReadOnly);
    if (!opened)
        return {};

    QJsonParseError parse_error;
    auto doc = QJsonDocument::fromJson(db_file.readAll(), &parse_error);
    if (doc.isNull())
        return {};

    auto records = doc.object();
    if (records.isEmpty())
        return {};

    std::unordered_map<std::string, mp::VMSpecs> reconstructed_records;
    for (auto it = records.constBegin(); it != records.constEnd(); ++it)
    {
        auto key = it.key().toStdString();
        auto record = it.value().toObject();
        if (record.isEmpty())
            return {};

        auto num_cores = record["num_cores"].toInt();
        auto mem_size = record["mem_size"].toString();
        auto disk_space = record["disk_space"].toInt();

        reconstructed_records[key] = {num_cores, mem_size.toStdString(), static_cast<size_t>(disk_space)};
    }
    return reconstructed_records;
}

auto fetch_image_for(const std::string& name, const mp::FetchType& fetch_type, mp::VMImageVault& vault)
{
    auto stub_prepare = [](const mp::VMImage&) -> mp::VMImage { return {}; };
    auto stub_progress = [](int progress) {};

    mp::Query query;
    query.name = name;

    return vault.fetch_image(fetch_type, query, stub_prepare, stub_progress);
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
          QObject::connect(&daemon_rpc, &DaemonRpc::on_ssh_info, daemon, &Daemon::ssh_info,
                           Qt::BlockingQueuedConnection);
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
    : config{std::move(the_config)},
      vm_instance_specs{load_db(config->cache_directory)},
      runner(config->server_address, this)
{
    std::vector<std::string> invalid_specs;
    for (auto const& entry : vm_instance_specs)
    {
        const auto& name = entry.first;
        const auto& spec = entry.second;

        if (!config->vault->has_record_for(name))
        {
            invalid_specs.push_back(name);
            continue;
        }

        auto vm_image = fetch_image_for(name, config->factory->fetch_type(), *config->vault);
        mp::VirtualMachineDescription vm_desc{spec.num_cores, spec.mem_size, spec.disk_space, name, vm_image};
        vm_instances[name] = config->factory->create_virtual_machine(vm_desc, *this);
    }

    for (const auto& bad_spec : invalid_specs)
    {
        vm_instance_specs.erase(bad_spec);
    }

    if (!invalid_specs.empty())
        persist_instances();
}

grpc::Status mp::Daemon::create(grpc::ServerContext* context, const CreateRequest* request,
                                grpc::ServerWriter<CreateReply>* server) // clang-format off
try // clang-format on
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
    auto cloud_init_config = make_cloud_init_config(*config->ssh_key_provider);
    auto vm_desc = to_machine_desc(request, name, vm_image, std::move(cloud_init_config));

    auto vm = config->factory->create_virtual_machine(vm_desc, *this);
    vm->start();
    vm->wait_until_ssh_up(std::chrono::minutes(5));

    vm_instances[name] = std::move(vm);
    vm_instance_specs[name] = {vm_desc.num_cores, vm_desc.mem_size, vm_desc.disk_space};
    persist_instances();

    CreateReply reply;
    reply.set_create_complete("Create setup complete.");
    server->Write(reply);

    reply.set_vm_instance_name(name);
    server->Write(reply);

    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::empty_trash(grpc::ServerContext* context, const EmptyTrashRequest* request,
                                     EmptyTrashReply* response) // clang-format off
try //clang-format on
{
    std::vector<decltype(vm_instance_trash)::key_type> keys_to_delete;
    for (auto& trash : vm_instance_trash)
    {
        const auto& name = trash.first;
        config->vault->remove(name);
        keys_to_delete.push_back(name);
    }

    for (auto const& key : keys_to_delete)
    {
        vm_instance_trash.erase(key);
        vm_instance_specs.erase(key);
    }

    persist_instances();
    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::exec(grpc::ServerContext* context, const ExecRequest* request, ExecReply* response) // clang-format off
try //clang-format on
{
    const auto name = request->instance_name();
    auto it = vm_instances.find(name);
    if (it == vm_instances.end())
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" does not exist", "");
    }

    auto port = it->second->forwarding_port();

    if (request->command_line_args_size() == 0)
        return grpc::Status(grpc::StatusCode::UNIMPLEMENTED, "", "");

    std::vector<std::string> cmd_line;
    for (const auto& arg : request->command_line_args())
    {
        cmd_line.push_back(arg);
    }

    mp::SSHSession session{port, *config->ssh_key_provider};
    auto result = session.execute(cmd_line);
    for (auto& arg : cmd_line)
    {
        response->add_exec_line(std::move(arg));
    }

    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::info(grpc::ServerContext* context, const InfoRequest* request, InfoReply* response) // clang-format off
try //clang-format on
{
    const auto name = request->instance_name();
    auto it = vm_instances.find(name);
    bool in_trash{false};
    if (it == vm_instances.end())
    {
        it = vm_instance_trash.find(name);
        if (it == vm_instance_trash.end())
            return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" does not exist", "");
        in_trash = true;
    }

    auto vm_image = fetch_image_for(name, config->factory->fetch_type(), *config->vault);
    auto& vm = it->second;
    response->set_name(name);
    if (in_trash)
    {
        response->set_status(InfoReply::TRASHED);
    }
    else
    {
        auto status_for = [](mp::VirtualMachine::State state) {
            switch (state)
            {
            case mp::VirtualMachine::State::running:
                return InfoReply::RUNNING;
            default:
                return InfoReply::STOPPED;
            }
        };
        response->set_status(status_for(vm->current_state()));
    }

    auto vm_image_info = config->image_host->info_for({"", vm_image.id, false});
    response->set_release(vm_image_info.release_title.toStdString());
    response->set_id(vm_image.id);

    // TODO: fill in IP, memory usage, load, disk_usage
    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::list(grpc::ServerContext* context, const ListRequest* request, ListReply* response) // clang-format off
try //clang-format on
{
    auto status_for = [](mp::VirtualMachine::State state) {
        switch (state)
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

    for (const auto& instance : vm_instance_trash)
    {
        const auto& name = instance.first;
        auto entry = response->add_instances();
        entry->set_name(name);
        entry->set_status(mp::ListVMInstance::TRASHED);
    }

    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::recover(grpc::ServerContext* context, const RecoverRequest* request, RecoverReply* response) // clang-format off
try //clang-format on
{
    const auto name = request->instance_name();
    auto it = vm_instance_trash.find(name);
    if (it == vm_instance_trash.end())
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" does not exist", "");
    }

    vm_instances[it->first] = std::move(it->second);
    vm_instance_trash.erase(name);
    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::ssh_info(grpc::ServerContext* context, const SSHInfoRequest* request, SSHInfoReply* response) // clang-format off
try //clang-format on
{
    const auto name = request->instance_name();
    auto it = vm_instances.find(name);
    if (it == vm_instances.end())
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" does not exist", "");
    }

    response->set_port(it->second->forwarding_port());
    response->set_priv_key_base64(config->ssh_key_provider->private_key_as_base64());
    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}


grpc::Status mp::Daemon::start(grpc::ServerContext* context, const StartRequest* request, StartReply* response) // clang-format off
try //clang-format on
{
    const auto name = request->instance_name();
    auto it = vm_instances.find(name);
    if (it == vm_instances.end())
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" does not exist", "");
    }

    it->second->start();
    it->second->wait_until_ssh_up(std::chrono::minutes(2));
    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::stop(grpc::ServerContext* context, const StopRequest* request, StopReply* response) // clang-format off
try //clang-format on
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
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
}

grpc::Status mp::Daemon::trash(grpc::ServerContext* context, const TrashRequest* request, TrashReply* response) // clang-format off
try //clang-format on
{
    const auto name = request->instance_name();
    auto it = vm_instances.find(name);
    if (it == vm_instances.end())
    {
        return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, "instance \"" + name + "\" does not exist", "");
    }

    it->second->shutdown();
    vm_instance_trash[it->first] = std::move(it->second);
    vm_instances.erase(name);
    return grpc::Status::OK;
}
catch (const std::exception& e)
{
    return grpc::Status(grpc::StatusCode::FAILED_PRECONDITION, e.what(), "");
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

void mp::Daemon::persist_instances()
{
    auto vm_spec_to_json = [](const mp::VMSpecs& specs) {
        QJsonObject json;
        json.insert("num_cores", specs.num_cores);
        json.insert("mem_size", QString::fromStdString(specs.mem_size));
        json.insert("disk_space", static_cast<int>(specs.disk_space));
        return json;
    };
    QJsonObject instance_records_json;
    for (const auto& record : vm_instance_specs)
    {
        auto key = QString::fromStdString(record.first);
        instance_records_json.insert(key, vm_spec_to_json(record.second));
    }
    QDir cache_dir{config->cache_directory};
    mp::write_json(instance_records_json, cache_dir.filePath(instance_db_name));
}
