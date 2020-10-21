
load("@com_github_grpc_grpc//bazel:grpc_deps.bzl", "grpc_deps")
load("@rules_proto_grpc//:repositories.bzl", "rules_proto_grpc_toolchains")




def clean_dep(dep):
    return str(Label(dep))

def deps_build_all():
  	grpc_deps()
  	rules_proto_grpc_toolchains()
