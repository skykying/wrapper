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
#include "default_vm_image_vault.h"
#include "ubuntu_image_host.h"

#include <multipass/name_generator.h>
#include <multipass/platform.h>
#include <multipass/ssh_key.h>
#include <multipass/virtual_machine_execute.h>

#include <QStandardPaths>

namespace mp = multipass;
std::unique_ptr<const mp::DaemonConfig> mp::DaemonConfigBuilder::build()
{
    if (url_downloader == nullptr)
        url_downloader = std::make_unique<URLDownloader>();
    if (factory == nullptr)
        factory = Platform::vm_backend();
    if (cache_directory.isEmpty())
        cache_directory = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (image_host == nullptr)
        image_host =
            std::make_unique<mp::UbuntuVMImageHost>("http://cloud-images.ubuntu.com/releases/", "streams/v1/index.json",
                                                    url_downloader.get(), std::chrono::minutes{5});
    if (vault == nullptr)
        vault = std::make_unique<DefaultVMImageVault>(image_host.get(), url_downloader.get(), cache_directory);
    if (name_generator == nullptr)
        name_generator = mp::make_default_name_generator();
    if (vm_execute == nullptr)
        vm_execute = Platform::vm_execute();
    if (server_address.empty())
        server_address = Platform::default_server_address();
    if (ssh_key == nullptr)
        ssh_key = Platform::public_key();

    return std::unique_ptr<const DaemonConfig>(new DaemonConfig{
        std::move(url_downloader), std::move(factory), std::move(image_host), std::move(vault),
        std::move(name_generator), std::move(vm_execute), std::move(ssh_key), cache_directory, server_address});
}
