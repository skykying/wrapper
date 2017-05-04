/*
 * Copyright (C) 2017 Canonical, Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>
 *
 */

#ifndef MULTIPASS_VIRTUAL_MACHINE_H
#define MULTIPASS_VIRTUAL_MACHINE_H

#include <memory>

namespace multipass
{
class VirtualMachine
{
public:
    enum class State
    {
        off,
        stopped,
        running
    };

    using UPtr = std::unique_ptr<VirtualMachine>;

    virtual ~VirtualMachine() = default;
    virtual void stop() = 0;
    virtual void start() = 0;
    virtual void shutdown() = 0;
    virtual State current_state() = 0;

protected:
    VirtualMachine() = default;
    VirtualMachine(const VirtualMachine&) = delete;
    VirtualMachine& operator=(const VirtualMachine&) = delete;
};
}
#endif // MULTIPASS_VIRTUAL_MACHINE_H
