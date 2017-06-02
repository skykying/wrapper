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

#include "qemu_virtual_machine_execute.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QStandardPaths>
#include <QString>

namespace mp = multipass;

namespace {
auto construct_ssh_command()
{
    const QString id_rsa_name("id_rsa");
    QString id_rsa_path(QStandardPaths::locate(QStandardPaths::CacheLocation, id_rsa_name));

    if (!QFile::exists(id_rsa_path))
    {
        id_rsa_path = QDir(QCoreApplication::applicationDirPath()).filePath(id_rsa_name);
    }

    // The following ssh command will undoubtedly change in The Future
    QString ssh_cmd("ssh -p 2222 ubuntu@localhost");

    if (QFile::exists(id_rsa_path))
    {
        ssh_cmd = "ssh -p 2222 -i " + id_rsa_path + " ubuntu@localhost";
    }

    return ssh_cmd.toStdString();
}
}

std::string mp::QemuVirtualMachineExecute::execute()
{
    return construct_ssh_command();
}

std::string mp::QemuVirtualMachineExecute::execute(std::string command)
{
    return construct_ssh_command() + " " + command;
}
