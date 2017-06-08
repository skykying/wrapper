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
#ifndef MULTIPASS_SSH_KEY_H
#define MULTIPASS_SSH_KEY_H

#include <string>

namespace multipass
{
class SshPubKey
{
public:
    enum class Type
    {
        RSA
    };

    virtual Type type() const = 0;
    virtual std::string as_base64() const = 0;
};
}

#endif // MULTIPASS_SSH_KEY_H
