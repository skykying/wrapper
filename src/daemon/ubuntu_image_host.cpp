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

namespace mp = multipass;

mp::VMImage mp::UbuntuVMImageHost::fetch(VMImageQuery const& query)
{
    VMImage image_info;

    image_info.image_path = ss_mgr.download_image_by_hash(query.query_string);
    return image_info;
}

void mp::UbuntuVMImageHost::update_image_manifest()
{
    ss_mgr.update_ss_manifest();
}

std::string mp::UbuntuVMImageHost::get_image_hash_for_query(std::string query_string)
{
    return ss_mgr.get_image_hash(query_string);
}
