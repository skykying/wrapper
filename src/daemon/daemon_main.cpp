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

#include "daemon.h"
#include "daemon_config.h"

#include <multipass/platform.h>
#include <multipass/virtual_machine_factory.h>
#include <multipass/vm_image_host.h>
#include <multipass/vm_image_vault.h>

#include <exception>

int main(int argc, char* argv[]) try
{
    multipass::DaemonConfig config;
    multipass::Daemon daemon(config);
    daemon.run();
}
catch (const std::exception& e)
{
    std::cerr << "Error: " << e.what() << std::endl;
}
