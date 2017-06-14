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

#include <multipass/platform.h>
#include <multipass/virtual_machine.h>
#include <multipass/virtual_machine_description.h>
#include <src/platform/backends/qemu/qemu_virtual_machine_execute.h>
#include <src/platform/backends/qemu/qemu_virtual_machine_factory.h>

#include "mock_status_monitor.h"
#include "stub_status_monitor.h"

#include <experimental/optional>
#include <system_error>

#include <QtCore/QTemporaryDir>
#include <gmock/gmock.h>

namespace mp = multipass;

using namespace testing;

struct QemuBackend : public testing::Test
{
    mp::VirtualMachineDescription default_description{2, 1024 * 1024 * 1024};
    mp::QemuVirtualMachineFactory backend;
};

TEST_F(QemuBackend, creates_in_running_state)
{
    StubVMStatusMonitor stub_monitor;
    auto machine = backend.create_virtual_machine(default_description, stub_monitor);
    EXPECT_THAT(machine->current_state(), Eq(mp::VirtualMachine::State::running));
}

TEST_F(QemuBackend, machine_sends_monitoring_events)
{
    mp::QemuVirtualMachineFactory backend;
    MockVMStatusMonitor mock_monitor;

    auto machine = backend.create_virtual_machine(default_description, mock_monitor);

    EXPECT_CALL(mock_monitor, on_stop());
    machine->stop();

    EXPECT_CALL(mock_monitor, on_resume());
    machine->start();

    EXPECT_CALL(mock_monitor, on_shutdown());
    machine->shutdown();
}

TEST_F(QemuBackend, execute_mangles_command)
{
    mp::QemuVirtualMachineExecute vm_execute;

    auto cmd_line = vm_execute.execute("foo");

    EXPECT_THAT(cmd_line, Eq("ssh -p 2222 ubuntu@localhost foo"));
}

TEST_F(QemuBackend, execute_ssh_only_no_command)
{
    mp::QemuVirtualMachineExecute vm_execute;

    auto cmd_line = vm_execute.execute();

    EXPECT_THAT(cmd_line, Eq("ssh -p 2222 ubuntu@localhost"));
}

namespace
{
class TemporaryEnvironmentVariable
{
public:
    TemporaryEnvironmentVariable(std::string const& key, std::string const& value)
        : key{key}, original_value{[](std::string const& key) -> std::experimental::optional<std::string> {
              const auto val = getenv(key.c_str());
              if (val)
              {
                  return std::string{val};
              }
              return {};
          }(key)}
    {
        if (setenv(key.c_str(), value.c_str(), true))
        {
            throw std::system_error{errno, std::system_category(), "Failed to set environment variable"};
        }
    }

    ~TemporaryEnvironmentVariable() noexcept(false)
    {
        if (original_value)
        {
            if (setenv(key.c_str(), original_value->c_str(), true))
            {
                throw std::system_error{errno, std::system_category(), "Failed to reset environment variable"};
            }
        }
        else
        {
            if (unsetenv(key.c_str()))
            {
                throw std::system_error{errno, std::system_category(),
                                        "Failed to unset temporary environment variable"};
            }
        }
    }

private:
    std::string const key;
    std::experimental::optional<std::string> const original_value;
};
}

TEST_F(QemuBackend, public_key_is_stable)
{
    QTemporaryDir fake_config_dir;
    TemporaryEnvironmentVariable cfg_override{"XDG_CONFIG_HOME", fake_config_dir.path().toStdString()};

    const auto key_one = mp::Platform::public_key()->as_base64();
    const auto key_two = mp::Platform::public_key()->as_base64();

    EXPECT_THAT(key_one, StrEq(key_two));
}

TEST_F(QemuBackend, uses_public_key_from_xdg_config_dir)
{
    QTemporaryDir fake_config_dir;
    TemporaryEnvironmentVariable cfg_override{"XDG_DATA_HOME", fake_config_dir.path().toStdString()};

    QDir fake_config_path{fake_config_dir.path()};
    fake_config_path.mkdir("multipassd");

    QFile fake_id_rsa{fake_config_path.filePath("multipassd/id_rsa.pub")};
    if (!fake_id_rsa.open(QIODevice::WriteOnly))
    {
        throw std::runtime_error{"Failed to create mock id_rsa.pub"};
    }
    const auto key_type = "ssh-rsa";
    const auto key = "AAAAB3NzaC1yc2EAAAADAQABAAABAQCj2HRELDuoAtglyqhOIHtT47gYbD773flgdigeqS+Qcf+"
                     "EAPRr2qdyfIYnGLbk22GmBQhKyhXy8YqQLxoPlXzzdV6dZ8AriPnqfH38gIYljXSdy+PbN7OyWNcsENpE1LKhkADtmMQc+"
                     "N0GffSwXFt7a8cgzNRsDDa7mOhAxS6Q5xFtANdZGWa75gk9UM04hYb9w4ZbSCtMhcS7okYM60UeydbgkA6ZjD7+"
                     "AyaQJ06cwlMQIV5o6Kp4EpLzXrvsnBS5Ej50811sz5KHrCeiwxG3YhyCZzSX5L67HepVLxdyb9E+kLOWNzPePnO2hAASDG+"
                     "2vsxt6L7OUOnish87mbGT";

    fake_id_rsa.write(key_type);
    fake_id_rsa.write(" ");
    fake_id_rsa.write(key);
    fake_id_rsa.write(" comment@localhost");

    fake_id_rsa.close();

    const auto parsed_key = mp::Platform::public_key();

    EXPECT_THAT(parsed_key->type(), Eq(mp::SshPubKey::Type::RSA));
    EXPECT_THAT(parsed_key->as_base64(), StrEq(key));
}

TEST_F(QemuBackend, creates_new_pubkey_when_none_exists)
{
    QTemporaryDir fake_config_dir;
    TemporaryEnvironmentVariable cfg_override{"XDG_DATA_HOME", fake_config_dir.path().toStdString()};

    QDir fake_config_path{fake_config_dir.path()};
    fake_config_path.mkdir("multipassd");

    const auto id_rsa_path = fake_config_path.filePath("multipassd/id_rsa.pub");
    ASSERT_FALSE(QFile::exists(id_rsa_path));

    const auto parsed_key = mp::Platform::public_key();

    EXPECT_TRUE(QFile::exists(id_rsa_path));
    EXPECT_THAT(parsed_key->type(), Eq(mp::SshPubKey::Type::RSA));
    EXPECT_THAT(parsed_key->as_base64(), Not(StrEq("")));
}
