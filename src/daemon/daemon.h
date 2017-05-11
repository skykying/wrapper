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

#include "daemon_config.h"
#include <grpc++/grpc++.h>
#include <multipass/virtual_machine.h>
#include <multipass/vm_status_monitor.h>
#include <multipass/rpc/multipass.grpc.pb.h>

#include <memory>

namespace multipass
{
class DaemonConfig;
class Daemon : public multipass::Rpc::Service, public multipass::VMStatusMonitor
{
public:
    Daemon(DaemonConfig config);
    void run();
    void shutdown();

private:
    DaemonConfig config;
    std::unique_ptr<grpc::Server> server;
    std::vector<VirtualMachine::UPtr> vms;

    grpc::Status launch(grpc::ServerContext* context, const LaunchRequest* request,
                        LaunchReply* reply) override;

    grpc::Status ssh(grpc::ServerContext* context, const SSHRequest* request,
                     SSHReply* response) override;

    grpc::Status start(grpc::ServerContext* context, const StartRequest* request,
                       StartReply* response) override;

    grpc::Status stop(grpc::ServerContext* context, const StopRequest* request,
                      StopReply* response) override;

    grpc::Status destroy(grpc::ServerContext* context, const DestroyRequest* request,
                         DestroyReply* response) override;

    grpc::Status list(grpc::ServerContext* context, const ListRequest* request,
                      ListReply* response) override;

    grpc::Status version(grpc::ServerContext* context, const VersionRequest* request,
                         VersionReply* response) override;

    void on_resume() override;
    void on_stop() override;
    void on_shutdown() override;

    Daemon(const Daemon&) = delete;
    Daemon& operator=(const Daemon&) = delete;
};
}
#endif // MULTIPASS_DAEMON_H
