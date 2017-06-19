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

#ifndef MULTIPASS_STUB_VM_IMAGE_FETCHER_H
#define MULTIPASS_STUB_VM_IMAGE_FETCHER_H

#include <multipass/vm_image_fetcher.h>

struct StubVMImageFetcher final : public multipass::VMImageFetcher
{
    multipass::VMImage fetch(const multipass::VMImageQuery&) final
    {
        return multipass::VMImage();
    }
};

#endif // MULTIPASS_STUB_VM_IMAGE_FETCHER_H
