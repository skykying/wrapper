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
#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QProcess>
#include <QtCore/QStandardPaths>
#include <QtCore/QTextStream>

namespace mp = multipass;

namespace
{
class OpenSSHPubKey : public mp::SshPubKey
{
public:
    OpenSSHPubKey(QTextStream& reader) : type_{parse_type(reader)}, base64_key{read_key_data(reader)}
    {
    }

    Type type() const override
    {
        return Type::RSA;
    }

    std::string as_base64() const override
    {
        return base64_key;
    }

private:
    static Type parse_type(QTextStream& reader)
    {
        QString assumed_to_be_rsa;
        reader >> assumed_to_be_rsa;

        return Type::RSA;
    }
    static std::string read_key_data(QTextStream& reader)
    {
        QString key_data;
        reader >> key_data;
        return key_data.toStdString();
    }

    Type const type_;
    std::string const base64_key;
};

void create_keypair(const QString& basename)
{
    QProcess ssh_keygen;
    QStringList keygen_args;

    keygen_args << "-t"
                << "rsa";
    keygen_args << "-f" << basename;
    keygen_args << "-N"
                << "";

    ssh_keygen.start("ssh-keygen", keygen_args);
    ssh_keygen.waitForFinished();
}
}

std::unique_ptr<mp::SshPubKey> mp::OpenSSHKeyProvider::public_key()
{
    QCoreApplication::setApplicationName("multipassd");
    const auto key_path = []() {
        auto const path = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "id_rsa.pub");
        if (path.isEmpty())
        {
            // QStandardPaths returns empty string on file-not-found
            return QDir{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)}.filePath("id_rsa.pub");
        }
        return path;
    }();

    if (!QFile::exists(key_path))
    {
        const QDir cfg_dir{QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation)};
        if (!cfg_dir.exists())
        {
            if (!cfg_dir.mkpath(cfg_dir.absolutePath()))
            {
                throw std::runtime_error{std::string{"Failed to create multipassd config dir"}};
            }
        }
        create_keypair(QDir{cfg_dir}.filePath("id_rsa"));
        if (!QFile::exists(key_path))
        {
            throw std::runtime_error{"Failed to create multipassd SSH keypair"};
        }
    }

    QFile key{key_path};
    if (!key.open(QIODevice::ReadOnly))
    {
        throw std::runtime_error{std::string{"Failed to open SSH public key file: "} + key.errorString().toStdString()};
    }
    QTextStream keydata{key.readAll()};
    return std::make_unique<OpenSSHPubKey>(keydata);
}
