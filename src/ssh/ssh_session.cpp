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

#include <multipass/ssh/ssh_session.h>

#include <stdexcept>

namespace mp = multipass;

mp::SSHSession::SSHSession(const std::string& host, int port) : session{ssh_new(), ssh_free}
{
    ssh_options_set(session.get(), SSH_OPTIONS_HOST, host.c_str());
    ssh_options_set(session.get(), SSH_OPTIONS_PORT, &port);
    auto ret = ssh_connect(session.get());
    if (ret != SSH_OK)
        throw std::runtime_error(ssh_get_error(session.get()));
}

mp::SSHSession::SSHSession(int port) : SSHSession("localhost", port)
{
}
