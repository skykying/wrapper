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

project(Multipass)
cmake_minimum_required(VERSION 3.0)

# Needs to be here before we set our own compilation options
add_subdirectory(3rd-party)

option(MULTIPASS_ENABLE_TESTS "Build tests" ON)
add_compile_options(-std=c++14 -Werror -Wall -pedantic)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# TODO: pull this in tree instead of relying on pkg-config
find_package(PkgConfig)
pkg_check_modules(PROCESS_CPP process-cpp REQUIRED)

set(MULTIPASS_GENERATED_SOURCE_DIR ${CMAKE_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${MULTIPASS_GENERATED_SOURCE_DIR})

include_directories(
  include
  ${MULTIPASS_GENERATED_SOURCE_DIR}
  ${gRPC_INCLUDE_DIRS}
)

add_subdirectory(src)

if(MULTIPASS_ENABLE_TESTS)
    find_package(GMock)
    add_subdirectory(tests)
endif()
