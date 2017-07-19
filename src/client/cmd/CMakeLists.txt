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

if(MSVC)
  set(PLATFORM_COMMANDS connect.cpp exec.cpp)
else()
  set(PLATFORM_COMMANDS connect_unix.cpp exec_unix.cpp execute_helper.cpp)
endif()

add_library(commands STATIC
  create.cpp
  empty_trash.cpp
  help.cpp
  info.cpp
  list.cpp
  recover.cpp
  start.cpp
  stop.cpp
  trash.cpp
  version.cpp

  ${PLATFORM_COMMANDS})

target_link_libraries(commands
  rpc
  Qt5::Core)
