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

#ifndef MULTIPASS_SSH_H
#define MULTIPASS_SSH_H

#include <libssh/libssh.h>

#include <memory>
#include <string>
#include <vector>

namespace multipass
{
class SSHKeyProvider;
class SSHSession
{
public:
    SSHSession(int port);
    SSHSession(int port, const SSHKeyProvider& key_provider);

    std::vector<std::string> execute(const std::vector<std::string>& args);
private:
    SSHSession(int port, const SSHKeyProvider* key_provider);
    std::unique_ptr<ssh_session_struct, void(*)(ssh_session)> session;
};
}
#endif // MULTIPASS_SSH_H
