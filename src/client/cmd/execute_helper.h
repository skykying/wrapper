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
 * Authored by: Chris Townsend <christopher.townsend@canonical.com>
 *
 */

#ifndef MULTIPASS_EXECUTE_HELPER_H
#define MULTIPASS_EXECUTE_HELPER_H

#include <multipass/cli/argparser.h>

#include <string>
#include <vector>

namespace multipass
{
ReturnCode ssh_exec(int port, const std::string& priv_key_blob,
                    const std::vector<std::string>& args);
ReturnCode ssh_connect(int port, const std::string& priv_key_blob);
}

#endif // MULTIPASS_EXECUTE_HELPER_H
