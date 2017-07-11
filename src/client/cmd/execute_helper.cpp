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

#include "execute_helper.h"

#ifdef MULTIPASS_PLATFORM_LINUX
#include <unistd.h>
#endif

#include <sstream>
#include <vector>

namespace
{
auto parse_exec_line(std::string exec_line)
{
    std::istringstream iss(exec_line);

    std::vector<std::string> tokens{std::istream_iterator<std::string>{iss},
                                    std::istream_iterator<std::string>{}};

    return tokens;
}

auto to_argv(const std::vector<std::string>& v)
{
    std::vector<char*> result;
    for (const auto& s : v)
        result.push_back(const_cast<char*>(s.c_str()));
    result.push_back(nullptr);
    return result;
}
}

namespace multipass
{
ReturnCode execute_process(std::string exec_line)
{
    auto parsed_cmd = parse_exec_line(exec_line);
    auto cmd = to_argv(parsed_cmd);
#ifdef MULTIPASS_PLATFORM_LINUX
    return static_cast<ReturnCode>(execvp(cmd[0], cmd.data()));
#else
    return ReturnCode::Ok;
#endif
}
}
