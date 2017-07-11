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

#ifndef MULTIPASS_DAEMON_H
#define MULTIPASS_DAEMON_H

#include "auto_join_thread.h"
#include "daemon_config.h"
#include "daemon_rpc.h"
#include <multipass/virtual_machine.h>
#include <multipass/vm_status_monitor.h>

#include <memory>
#include <unordered_map>

namespace multipass
{
struct VMSpecs
{
    int num_cores;
    std::string mem_size;
    size_t disk_space;
};

class Daemon;
class DaemonRunner
{
public:
    DaemonRunner(const std::string& server_address, Daemon* daemon);
    ~DaemonRunner();

private:
    DaemonRpc daemon_rpc;
    AutoJoinThread daemon_thread;
};

struct DaemonConfig;
class Daemon : public QObject, public multipass::Rpc::Service, public multipass::VMStatusMonitor
{
    Q_OBJECT
public:
    Daemon(std::unique_ptr<const DaemonConfig> config);

protected:
    void on_resume() override;
    void on_stop() override;
    void on_shutdown() override;

public slots:
    grpc::Status create(grpc::ServerContext* context, const CreateRequest* request,
                        grpc::ServerWriter<CreateReply>* reply) override;

    grpc::Status empty_trash(grpc::ServerContext* context, const EmptyTrashRequest* request, EmptyTrashReply* response) override;

    grpc::Status exec(grpc::ServerContext* context, const ExecRequest* request, ExecReply* response) override;

    grpc::Status info(grpc::ServerContext* context, const InfoRequest* request, InfoReply* response) override;

    grpc::Status list(grpc::ServerContext* context, const ListRequest* request, ListReply* response) override;

    grpc::Status recover(grpc::ServerContext* context, const RecoverRequest* request, RecoverReply* response) override;

    grpc::Status start(grpc::ServerContext* context, const StartRequest* request, StartReply* response) override;

    grpc::Status stop(grpc::ServerContext* context, const StopRequest* request, StopReply* response) override;

    grpc::Status trash(grpc::ServerContext* context, const TrashRequest* request, TrashReply* response) override;

    grpc::Status version(grpc::ServerContext* context, const VersionRequest* request, VersionReply* response) override;

private:
    void persist_instances();
    std::unique_ptr<const DaemonConfig> config;
    std::unordered_map<std::string, VMSpecs> vm_instance_specs;
    std::unordered_map<std::string, VirtualMachine::UPtr> vm_instances;
    DaemonRunner runner;

    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;
};
}
#endif // MULTIPASS_DAEMON_H
