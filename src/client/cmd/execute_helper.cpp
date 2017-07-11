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

#include <QDir>
#include <QStandardPaths>
#include <QTemporaryFile>

#include <unistd.h>

namespace mp = multipass;
namespace
{
auto to_argv(const std::vector<std::string>& v)
{
    std::vector<char*> result;
    for (const auto& s : v)
        result.push_back(const_cast<char*>(s.c_str()));
    result.push_back(nullptr);
    return result;
}

void write_blob_to(const std::string& blob, QFile& file)
{
    if (!file.open(QIODevice::WriteOnly))
        throw std::runtime_error("Unable to open ssh key file");

    auto written = file.write(blob.c_str());
    if (written == -1)
        throw std::runtime_error("Failed to to ssh key");

    file.setPermissions(QFile::ReadOwner | QFile::WriteOwner);
    file.close();
}

auto ssh_cmd_for(const std::string& host, int port, const std::string& priv_key_path)
{
    // The following ssh command will undoubtedly change in The Future
    std::vector<std::string> ssh_cmd{"ssh",
                                     "-p",
                                     std::to_string(port),
                                     "-t",
                                     "-o",
                                     "StrictHostKeyChecking=no",
                                     "-o",
                                     "UserKnownHostsFile=/dev/null",
                                     "-i",
                                     priv_key_path,
                                     "ubuntu@" + host};

    return ssh_cmd;
}

// This is needed in order to make ssh's command parsing happy:/
auto escape_cmd_args_with_quotes(const std::vector<std::string>& command)
{
    std::vector<std::string> escaped_cmd_line;

    for (auto const& cmd : command)
    {
        escaped_cmd_line.push_back("'" + cmd + "'");
    }

    return escaped_cmd_line;
}
}

mp::ReturnCode mp::ssh_exec(const std::string& host, int port, const std::string& priv_key_blob,
                            const std::vector<std::string>& args)
{
    QFile file{QDir::temp().filePath("multipass-key")};
    write_blob_to(priv_key_blob, file);

    auto ssh_cmd = ssh_cmd_for(host, port, file.fileName().toStdString());

    if (!args.empty())
    {
        ssh_cmd.push_back("--");

        auto escaped_cmd = escape_cmd_args_with_quotes(args);
        ssh_cmd.insert(ssh_cmd.end(), escaped_cmd.begin(), escaped_cmd.end());
    }

    auto cmd = to_argv(ssh_cmd);
    return static_cast<ReturnCode>(execvp(cmd[0], cmd.data()));
}

mp::ReturnCode mp::ssh_connect(const std::string& host, int port, const std::string& priv_key_blob)
{
    return mp::ssh_exec(host, port, priv_key_blob, {});
}
