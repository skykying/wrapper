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

if(MSVC)
  list(APPEND BACKEND_TESTS test_hyperv_backend.cpp)
elseif(APPLE)
  list(APPEND BACKEND_TESTS test_hyperkit_backend.cpp)
else()
  list(APPEND BACKEND_TESTS test_qemu_backend.cpp)
endif()

add_executable(multipass_tests
  file_reader.cpp
  main.cpp
  test_cli_client.cpp
  test_daemon.cpp
  test_data_path.cpp
  test_image_vault.cpp
  test_petname.cpp
  test_simple_streams_index.cpp
  test_simple_streams_manifest.cpp
  test_ubuntu_image_host.cpp

  ${BACKEND_TESTS}
)

target_link_libraries(multipass_tests
  daemon
  client
  petname
  simplestreams

  # 3rd-party
  gmock
  yaml-cpp
)

add_test(multipass_tests ${EXECUTABLE_OUTPUT_PATH}/multipass_tests)

file(COPY test_data DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
