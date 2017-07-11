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

#include <multipass/ssh/ssh_key_provider.h>

#include <array>
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
        return {read_stream(StreamType::out), read_stream(StreamType::err)};
    }

private:
    enum class StreamType
    {
        out,
        err
    };

    std::string read_stream(StreamType type)
    {
        std::stringstream output;
        std::array<char, 256> buffer;
        int num_bytes{0};
        const bool is_std_err = type == StreamType::err ? true : false;
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

mp::SSHSession::SSHSession(const std::string& host, int port, const SSHKeyProvider* key_provider)
    : session{ssh_new(), ssh_free}
{
    if (session == nullptr)
        throw std::runtime_error("Could not allocate ssh session");

    throw_on_error(ssh_options_set, session, SSH_OPTIONS_HOST, host.c_str());
    throw_on_error(ssh_options_set, session, SSH_OPTIONS_PORT, &port);
    throw_on_error(ssh_options_set, session, SSH_OPTIONS_USER, "ubuntu");
    throw_on_error(ssh_connect, session);
    if (key_provider)
        throw_on_error(ssh_userauth_publickey, session, nullptr, key_provider->private_key());
}

mp::SSHSession::SSHSession(const std::string& host, int port, const SSHKeyProvider& key_provider)
    : SSHSession(host, port, &key_provider)
{
}

mp::SSHSession::SSHSession(const std::string& host, int port) : SSHSession(host, port, nullptr)
{
}

std::vector<std::string> mp::SSHSession::execute(const std::vector<std::string>& args)
{
    Channel channel{session.get()};
    return channel.exec(to_cmd(args));
}
