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

#ifndef QEMU_VM_IMAGE_FETCHER_H
#define QEMU_VM_IMAGE_FETCHER_H

#include <multipass/vm_image_fetcher.h>

namespace multipass
{
class VMImageHost;

class QemuVMImageFetcher : public VMImageFetcher
{
public:
    QemuVMImageFetcher(const std::unique_ptr<VMImageHost>& image_host);
    ~QemuVMImageFetcher() = default;

    VMImage fetch(const VMImageQuery& query) override;

private:
    const std::unique_ptr<VMImageHost>& image_host;
};

} // namespace multipass

#endif // QEMU_VM_IMAGE_FETCHER_H
