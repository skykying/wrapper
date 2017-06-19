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
 * Authored by: Gerry Boland <gerry.boland@canonical.com>
 */

#include "qemu_vm_image_fetcher.h"

#include <multipass/vm_image_host.h>

namespace mp = multipass;

mp::QemuVMImageFetcher::QemuVMImageFetcher(const std::unique_ptr<mp::VMImageHost>& image_host) : image_host(image_host)
{
}

mp::VMImage mp::QemuVMImageFetcher::fetch(const mp::VMImageQuery& query)
{
    return image_host->fetch(query);
}
