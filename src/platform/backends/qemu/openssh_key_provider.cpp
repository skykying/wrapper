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
#include <QtCore/QFile>
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
}

std::unique_ptr<mp::SshPubKey> mp::OpenSSHKeyProvider::public_key()
{
    QCoreApplication::setApplicationName("multipassd");
    const auto key_path = QStandardPaths::locate(QStandardPaths::AppConfigLocation, "id_rsa.pub");

    if (QFile::exists(key_path))
    {
        QFile key{key_path};
        if (!key.open(QIODevice::ReadOnly))
        {
            throw std::runtime_error{std::string{"Failed to open SSH public key file: "} +
                                     key.errorString().toStdString()};
        }
        QTextStream keydata{key.readAll()};
        return std::make_unique<OpenSSHPubKey>(keydata);
    }

    QByteArray fake_data{"ssh-rsa "
                         "AAAAB3NzaC1yc2EAAAADAQABAAABAQCj2HRELDuoAtglyqhOIHtT47gYbD773flgdigeqS+Qcf+"
                         "EAPRr2qdyfIYnGLbk22GmBQhKyhXy8YqQLxoPlXzzdV6dZ8AriPnqfH38gIYljXSdy+"
                         "PbN7OyWNcsENpE1LKhkADtmMQc+"
                         "N0GffSwXFt7a8cgzNRsDDa7mOhAxS6Q5xFtANdZGWa75gk9UM04hYb9w4ZbSCtMhcS7okYM60UeydbgkA6ZjD7+"
                         "AyaQJ06cwlMQIV5o6Kp4EpLzXrvsnBS5Ej50811sz5KHrCeiwxG3YhyCZzSX5L67HepVLxdyb9E+"
                         "kLOWNzPePnO2hAASDG+2vsxt6L7OUOnish87mbGT"};
    QTextStream fake_reader{fake_data};
    return std::make_unique<OpenSSHPubKey>(fake_reader);
}
