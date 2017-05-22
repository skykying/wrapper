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

if(MSVC AND VCPKG_TOOLCHAIN)
  # https://github.com/google/googletest/issues/799
  add_definitions(-DGTEST_LANG_CXX11=1)
  add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY=1 /wd4251 /wd4275) # mask some warnings
  find_package(GTest REQUIRED)
  # GMock library fails to be found by vcpkg, workaround
  string(REPLACE "gtest" "gmock" GMOCK_LIBRARIES ${GTEST_LIBRARIES})
else()
  find_package(GMock REQUIRED)
endif()

include_directories(
  ${CMAKE_SOURCE_DIR}
  ${GMOCK_INCLUDE_DIRS}
  ${GTEST_INCLUDE_DIRS}
)

add_executable(multipass_tests
  main.cpp
  test_daemon.cpp
  test_qemu_backend.cpp
  test_petname.cpp
  test_simplestreams.cpp
  )

target_link_libraries(multipass_tests
  daemon
  client
  petname
  simplestreams
  ${GTEST_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(multipass_tests ${EXECUTABLE_OUTPUT_PATH}/multipass_tests)

if(NOT MSVC)
  add_dependencies(multipass_tests GMock)
endif()

file(COPY test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
