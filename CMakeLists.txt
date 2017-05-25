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

cmake_minimum_required(VERSION 3.1)

project(Multipass)

option(MULTIPASS_ENABLE_TESTS "Build tests" ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Qt config
set(CMAKE_AUTOMOC ON)
find_package(Qt5Core)
find_package(Qt5Network)

# Needs to be here before we set our own compilation options
add_subdirectory(3rd-party)

file(STRINGS ${CMAKE_SOURCE_DIR}/VERSION MULTIPASS_VERSION
  LIMIT_COUNT 1)

set(CMAKE_CXX_STANDARD 14)
if(MSVC)
  # Build with multiple processes
  add_definitions(/MP)
  add_definitions(-D_WIN32_WINNT=0x600) # A gprc header requires this (specifies WinXP or greater)
  add_definitions(-DMULTIPASS_PLATFORM_WINDOWS)
else()
  add_compile_options(-Werror -Wall -pedantic -fPIC)
  add_definitions(-DMULTIPASS_PLATFORM_POSIX)
endif()

set(MULTIPASS_GENERATED_SOURCE_DIR ${CMAKE_BINARY_DIR}/gen)
file(MAKE_DIRECTORY ${MULTIPASS_GENERATED_SOURCE_DIR})

include_directories(
  include
  ${MULTIPASS_GENERATED_SOURCE_DIR}
  ${gRPC_INCLUDE_DIRS}
  ${wrap_text_INCLUDE_DIRS}
  ${yaml_cpp_INCLUDE_DIRS}
)

add_subdirectory(src)

if(MULTIPASS_ENABLE_TESTS)
    add_subdirectory(tests)
endif()
