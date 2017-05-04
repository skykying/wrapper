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

#ifndef MULTIPASS_VM_IMAGE_REPOSITORY_H
#define MULTIPASS_VM_IMAGE_REPOSITORY_H

#include <multipass/vm_image_vault.h>

namespace multipass
{
class VMImageRepository final : public VMImageVault
{
public:
    void add_vm_image(VMImage image) override;
    VMImage find_image(const VaultQuery& query) override;
    void for_each_image_do(const Action& action) override;
};
}
#endif // MULTIPASS_VM_IMAGE_REPOSITORY_H
