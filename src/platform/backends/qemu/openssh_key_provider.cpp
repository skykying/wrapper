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
 * Authored by: Christopher James Halse Rogers <christopher.halse.rogers@canonical.com>
 *
 */

#include "openssh_key_provider.h"

namespace mp = multipass;

namespace
{
class OpenSSHPubKey : public mp::SshPubKey
{
public:
    Type type() const override
    {
        return Type::RSA;
    }

    std::string as_base64() const override
    {
        return "hello";
    }
};
}

std::unique_ptr<mp::SshPubKey> mp::OpenSSHKeyProvider::public_key()
{
    return std::make_unique<OpenSSHPubKey>();
}
