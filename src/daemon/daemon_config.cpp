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

#include "daemon_config.h"

#include "ubuntu_image_host.h"
#include "vm_image_repository.h"

#include <multipass/platform.h>
#include <multipass/name_generator.h>

namespace mp = multipass;

mp::DaemonConfig::DaemonConfig() : DaemonConfig(Platform::default_server_address()) {}

mp::DaemonConfig::DaemonConfig(std::string address)
    : DaemonConfig(Platform::vm_backend(), std::make_unique<mp::UbuntuVMImageHost>(),
                   std::make_unique<VMImageRepository>(), address)
{
}

mp::DaemonConfig::DaemonConfig(std::unique_ptr<VirtualMachineFactory> factory, std::unique_ptr<VMImageHost> image_host,
                               std::unique_ptr<VMImageVault> vault, std::string address)
    : factory{std::move(factory)}, image_host{std::move(image_host)}, vault{std::move(vault)},
      name_generator{mp::make_default_name_generator()}, server_address{address}
{
}
