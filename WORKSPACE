workspace(name = "wrapper")



load("//bazel:deps_setup.bzl", "deps_setup")

deps_setup()

load("//bazel:deps_build_all.bzl", "deps_build_all")

deps_build_all()

# This needs to be run after grpc_deps() in ray_deps_build_all() to make
# sure all the packages loaded by grpc_deps() are available. However a
# load() statement cannot be in a function so we put it here.
load("@com_github_grpc_grpc//bazel:grpc_extra_deps.bzl", "grpc_extra_deps")

grpc_extra_deps()