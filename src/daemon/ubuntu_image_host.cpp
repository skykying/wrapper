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

#include "ubuntu_image_host.h"

#include <multipass/query.h>
#include <multipass/simple_streams_index.h>
#include <multipass/url_downloader.h>

#include <QUrl>

namespace mp = multipass;

namespace
{
constexpr auto index_path = "streams/v1/index.json";

auto download_manifest(const QString& host_url, mp::URLDownloader* url_downloader)
{
    auto json_index = url_downloader->download({host_url + index_path});
    auto index = mp::SimpleStreamsIndex::fromJson(json_index);

    auto json_manifest = url_downloader->download({host_url + index.manifest_path});
    return mp::SimpleStreamsManifest::fromJson(json_manifest);
}

mp::VMImageInfo with_location_fully_resolved(const QString& host_url, const mp::VMImageInfo& info)
{
    return {info.aliases,
            info.release,
            host_url + info.image_location,
            host_url + info.kernel_location,
            host_url + info.initrd_location,
            info.id};
}
}

mp::UbuntuVMImageHost::UbuntuVMImageHost(QString host_url, URLDownloader* downloader,
                                         std::chrono::seconds manifest_time_to_live)
    : manifest_time_to_live{manifest_time_to_live}, url_downloader{downloader}, host_url{host_url}
{
}

mp::VMImageInfo mp::UbuntuVMImageHost::info_for(const Query& query)
{
    update_manifest();
    auto key = QString::fromStdString(query.release);
    if (key.isEmpty())
        key = "default";
    auto it = manifest->image_records.find(key);
    if (it == manifest->image_records.end())
        throw std::runtime_error("unable to find query on manifest");

    const auto info = it.value();
    return with_location_fully_resolved(host_url, *info);
}

void mp::UbuntuVMImageHost::for_each_entry_do(const Action& action)
{
    update_manifest();
    for (const auto& product : manifest->products)
    {
        action(with_location_fully_resolved(host_url, product));
    }
}

void mp::UbuntuVMImageHost::update_manifest()
{
    const auto now = std::chrono::steady_clock::now();
    if ((now - last_update) > manifest_time_to_live || manifest == nullptr)
    {
        manifest = download_manifest(host_url, url_downloader);
        last_update = now;
    }
}
