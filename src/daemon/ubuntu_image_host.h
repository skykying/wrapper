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

#ifndef MULTIPASS_UBUNTU_IMAGE_HOST_H
#define MULTIPASS_UBUNTU_IMAGE_HOST_H

#include <multipass/simple_streams_manifest.h>
#include <multipass/vm_image_host.h>

#include <chrono>

namespace multipass
{
class URLDownloader;
class UbuntuVMImageHost final : public VMImageHost
{
public:
    UbuntuVMImageHost(QString host_url, URLDownloader* downloader, std::chrono::seconds manifest_time_to_live);
    VMImageInfo info_for(const Query& query) override;
    void for_each_entry_do(const Action& action) override;

private:
    void update_manifest();
    std::chrono::seconds manifest_time_to_live;
    std::chrono::steady_clock::time_point last_update;
    std::unique_ptr<multipass::SimpleStreamsManifest> manifest;
    URLDownloader* const url_downloader;
    QString host_url;
    QString index_path;
};
}
#endif // MULTIPASS_UBUNTU_IMAGE_HOST_H
