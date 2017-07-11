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

#ifndef MULTIPASS_STUB_VM_IMAGE_VAULT_H
#define MULTIPASS_STUB_VM_IMAGE_VAULT_H

#include <multipass/vm_image_vault.h>

struct StubVMImageVault final : public multipass::VMImageVault
{
    multipass::VMImage fetch_image(const multipass::FetchType&, const multipass::Query&, const PrepareAction& prepare,
                                   const multipass::ProgressMonitor&) override
    {
        return prepare({});
    };

    void remove(const std::string& name) override{};
};

#endif // MULTIPASS_STUB_VM_IMAGE_VAULT_H
