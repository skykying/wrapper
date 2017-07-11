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

#include "src/daemon/ubuntu_image_host.h"
#include "test_data_path.h"

#include <multipass/query.h>
#include <multipass/url_downloader.h>

#include <QUrl>

#include <gmock/gmock.h>

#include <unordered_set>

namespace mp = multipass;
namespace mpt = multipass::test;

using namespace testing;

namespace
{
struct UbuntuImageHost : public testing::Test
{
    QString host_url{QUrl::fromLocalFile(mpt::test_data_path()).toString()};
    mp::URLDownloader url_downloader;
    std::chrono::seconds default_ttl{1};
    QString expected_location{host_url + "newest_image.img"};
    QString expected_id{"8842e7a8adb01c7a30cc702b01a5330a1951b12042816e87efd24b61c5e2239f"};
};
}

TEST_F(UbuntuImageHost, returns_expected_info)
{
    mp::UbuntuVMImageHost host{host_url, &url_downloader, default_ttl};

    mp::Query query{"", "xenial", false};
    auto info = host.info_for(query);

    EXPECT_THAT(info.image_location, Eq(expected_location));
    EXPECT_THAT(info.id, Eq(expected_id));
}

TEST_F(UbuntuImageHost, uses_default_on_unspecified_release)
{
    mp::UbuntuVMImageHost host{host_url, &url_downloader, default_ttl};

    mp::Query empty_query{"", "", false};
    auto info = host.info_for(empty_query);

    EXPECT_THAT(info.image_location, Eq(expected_location));
    EXPECT_THAT(info.id, Eq(expected_id));
}

TEST_F(UbuntuImageHost, iterates_over_all_entries)
{
    mp::UbuntuVMImageHost host{host_url, &url_downloader, default_ttl};

    std::unordered_set<std::string> ids;
    auto action = [&ids](const mp::VMImageInfo& info)
    {
        ids.insert(info.id.toStdString());
    };
    host.for_each_entry_do(action);

    const size_t expected_entries{4};
    EXPECT_THAT(ids.size(), Eq(expected_entries));

    EXPECT_THAT(ids.count("1797c5c82016c1e65f4008fcf89deae3a044ef76087a9ec5b907c6d64a3609ac"), Eq(1u));
    EXPECT_THAT(ids.count("8842e7a8adb01c7a30cc702b01a5330a1951b12042816e87efd24b61c5e2239f"), Eq(1u));
    EXPECT_THAT(ids.count("1507bd2b3288ef4bacd3e699fe71b827b7ccf321ec4487e168a30d7089d3c8e4"), Eq(1u));
    EXPECT_THAT(ids.count("0b115b83e7a8bebf3d3a02bf55ad0cb75a0ed515fcbc65fb0c9abe76c752921c"), Eq(1u));
}
