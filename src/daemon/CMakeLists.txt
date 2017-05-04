# Copyright © 2017 Canonical Ltd.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as
# published by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# Authored by: Alberto Aguirre <alberto.aguirre@canonical.com>

add_library(daemon STATIC
  daemon.cpp
  daemon_config.cpp
  ubuntu_image_host.cpp
  vm_image_repository.cpp)

target_link_libraries(daemon
  rpc
  platform)

add_executable(multipassd daemon_main.cpp)

target_link_libraries(multipassd
  daemon
  qemu_backend)
