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
  )

target_link_libraries(multipass_tests
  daemon
  client
  petname
  ${GTEST_LIBRARIES}
  ${GMOCK_LIBRARIES}
)

add_test(multipass_tests ${EXECUTABLE_OUTPUT_PATH}/multipass_tests)

add_dependencies(multipass_tests GMock)
