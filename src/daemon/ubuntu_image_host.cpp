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

#include <multipass/simplestreams.h>


namespace mp = multipass;

mp::VMImage mp::UbuntuVMImageHost::fetch(VMImageQuery const& query)
{
    mp::SimpleStreams ss_mgr;
    VMImage image_info;

    image_info.image_path = ss_mgr.download_image_by_alias(query.release);
    return image_info;
}
