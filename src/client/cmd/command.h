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

#ifndef MULTIPASS_COMMAND_H
#define MULTIPASS_COMMAND_H

#include <grpc++/grpc++.h>
#include <multipass/callable_traits.h>
#include <multipass/rpc/multipass.grpc.pb.h>

namespace multipass
{
namespace cmd
{
class Command
{
public:
    using UPtr = std::unique_ptr<Command>;
    Command(grpc::Channel& channel, Rpc::Stub& stub, std::ostream& cout, std::ostream& cerr)
        : rpc_channel{&channel}, stub{&stub}, cout{cout}, cerr{cerr}
    {
    }
    virtual ~Command() = default;
    virtual int run() = 0;
    virtual std::string name() const = 0;

protected:
    template <typename RpcFunc, typename Request, typename SuccessCallable, typename FailureCallable>
    int dispatch(RpcFunc&& rpc_func, const Request& request, SuccessCallable&& on_success, FailureCallable&& on_failure)
    {
        using SuccessCallableTraits = multipass::callable_traits<SuccessCallable>;
        using ReplyType = typename std::remove_reference<typename SuccessCallableTraits::template arg<0>::type>::type;
        using FailureCallableTraits = multipass::callable_traits<FailureCallable>;
        using FailureCallableArgType = typename FailureCallableTraits::template arg<0>::type;

        static_assert(SuccessCallableTraits::num_args == 1, "");
        static_assert(FailureCallableTraits::num_args == 1, "");
        static_assert(std::is_same<typename SuccessCallableTraits::return_type, int>::value, "");
        static_assert(std::is_same<typename FailureCallableTraits::return_type, int>::value, "");
        static_assert(std::is_same<FailureCallableArgType, grpc::Status&>::value, "");

        ReplyType reply;

        auto rpc_method =
            std::bind(rpc_func, stub, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);

        grpc::ClientContext context;
        auto status = rpc_method(&context, request, &reply);

        if (status.ok())
        {
            return on_success(reply);
        }

        return on_failure(status);
    }

    Command(const Command&) = delete;
    Command& operator=(const Command&) = delete;

    grpc::Channel* rpc_channel;
    Rpc::Stub* stub;
    std::ostream& cout;
    std::ostream& cerr;
};
}
}
#endif // MULTIPASS_COMMAND_H
