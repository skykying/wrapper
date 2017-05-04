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

set(MULTIPASS_PROTOCOL_SPEC multipass.proto)
protobuf_generate_cpp(GENERATED_PROTOBUF_SRCS GENERATED_PROTOBUF_HDRS ${MULTIPASS_PROTOCOL_SPEC})
grpc_generate_cpp(GENERATED_GRPC_SRCS GENERATED_GRPC_HDRS ${MULTIPASS_PROTOCOL_SPEC})

add_library(rpc STATIC
  ${GENERATED_PROTOBUF_SRCS}
  ${GENERATED_GRPC_SRCS}
  ${GENERATED_PROTOBUF_HDRS}
  ${GENERATED_GRPC_HDRS}
)

target_link_libraries(rpc
  gRPC::grpc++
  ${PROTOBUF_LIBRARIES})
