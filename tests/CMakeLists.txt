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

#if(MSVC AND VCPKG_TOOLCHAIN)
#  # https://github.com/google/googletest/issues/799
#  add_definitions(-DGTEST_LANG_CXX11=1)
#  add_definitions(-DGTEST_LINKED_AS_SHARED_LIBRARY=1 /wd4251 /wd4275) # mask some warnings
#  find_package(GTest REQUIRED)
  # GMock library fails to be found by vcpkg, workaround
#  string(REPLACE "gtest" "gmock" GMOCK_LIBRARIES ${GTEST_LIBRARIES})
#else()
#  find_package(GMock REQUIRED)
#endif()

# Prevent overriding the parent project's compiler/linker
# settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines
# the gmock/gtest and {gtest,gmock}_main targets.
add_subdirectory(
  ${CMAKE_SOURCE_DIR}/3rd-party/googletest
  ${CMAKE_BINARY_DIR}/3rd-party/googletest
  )

if(MSVC)
  add_definitions(-DGTEST_LANG_CXX11=1)
endif()

include_directories(
  ${gtest_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}
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

  # 3rd-party
  gmock
  )

add_test(multipass_tests ${EXECUTABLE_OUTPUT_PATH}/multipass_tests)

file(COPY test_data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
