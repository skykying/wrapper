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

#include <sstream>
#include <stdexcept>

namespace mp = multipass;

namespace
{
template <typename Callable, typename Handle, typename... Args>
void throw_on_error(Callable&& f, Handle&& h, Args&&... args)
{
    auto ret = f(h.get(), std::forward<Args>(args)...);
    if (ret != SSH_OK)
        throw std::runtime_error(ssh_get_error(h.get()));
}

auto quote_for(const std::string& arg)
{
    return arg.find('\'') == std::string::npos ? '\'' : '"';
}

std::string to_cmd(const std::vector<std::string>& args)
{
    std::stringstream cmd;
    for (auto const& arg : args)
    {
        const auto quote = quote_for(arg);
        cmd << quote << arg << quote << " ";
    }
    return cmd.str();
}

class Channel
{
public:
    Channel(ssh_session session) : channel{ssh_channel_new(session), ssh_channel_free}
    {
        throw_on_error(ssh_channel_open_session, channel);
    }

    std::vector<std::string> exec(const std::string& cmd)
    {
        throw_on_error(ssh_channel_request_exec, channel, cmd.c_str());
        return {read_stream(StreamType::stdout), read_stream(StreamType::stderr)};
    }

private:
    enum class StreamType
    {
        stdout,
        stderr
    };

    std::string read_stream(StreamType type)
    {
        std::stringstream output;
        std::array<char, 256> buffer;
        int num_bytes{0};
        const bool is_std_err = type == StreamType::stderr ? true : false;
        do
        {
            num_bytes = ssh_channel_read_timeout(channel.get(), buffer.data(), buffer.size(), is_std_err, -1);
            output.write(buffer.data(), num_bytes);
        } while (num_bytes > 0);

        auto result = output.str();
        return result;
    }
    std::unique_ptr<ssh_channel_struct, void (*)(ssh_channel)> channel;
};
}

mp::SSHSession::SSHSession(const std::string& host, int port, const std::string& priv_key_path)
    : session{ssh_new(), ssh_free}
{
    if (session == nullptr)
        throw std::runtime_error("Could not allocate ssh session");
    throw_on_error(ssh_options_set, session, SSH_OPTIONS_HOST, host.c_str());
    throw_on_error(ssh_options_set, session, SSH_OPTIONS_PORT, &port);
    throw_on_error(ssh_options_set, session, SSH_OPTIONS_USER, "ubuntu");
    throw_on_error(ssh_connect, session);

    if (!priv_key_path.empty())
    {
        ssh_key priv_key;
        auto imported = ssh_pki_import_privkey_file(priv_key_path.c_str(), nullptr, nullptr, nullptr, &priv_key);
        if (imported != SSH_OK)
            throw std::runtime_error("failed to import private key");
        std::unique_ptr<ssh_key_struct, void (*)(ssh_key)> key{priv_key, ssh_key_free};
        throw_on_error(ssh_userauth_publickey, session, nullptr, priv_key);
    }
}

mp::SSHSession::SSHSession(const std::string& host, int port) : SSHSession(host, port, "")
{
}

mp::SSHSession::SSHSession(int port) : SSHSession("localhost", port)
{
}

std::vector<std::string> mp::SSHSession::execute(const std::vector<std::string>& args)
{
    Channel channel{session.get()};
    return channel.exec(to_cmd(args));
}
