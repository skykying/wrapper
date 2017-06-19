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

#ifndef MULTIPASS_VM_IMAGE_FETCHER_H
#define MULTIPASS_VM_IMAGE_FETCHER_H

#include <multipass/virtual_machine.h>
#include <multipass/vm_image.h>

namespace multipass
{
class VMImageQuery;
class VMImageFetcher
{
public:
    virtual ~VMImageFetcher() = default;

    virtual VMImage fetch(const VMImageQuery& query) = 0;

protected:
    VMImageFetcher() = default;
    VMImageFetcher(const VMImageFetcher&) = delete;
    VMImageFetcher& operator=(const VMImageFetcher&) = delete;
};
}
#endif // MULTIPASS_VM_IMAGE_FETCHER_H
