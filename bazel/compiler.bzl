COPTS = ["-DRAY_USE_GLOG"] + select({
    "//:opt": ["-DBAZEL_OPT"],
    "//conditions:default": [],
}) + select({
    "@bazel_tools//src/conditions:windows": [
        # TODO(mehrdadn): (How to) support dynamic linking?
        "-DRAY_STATIC",
    ],
    "//conditions:default": [
    ],
}) + select({
    "//:clang-cl": [
        "-Wno-builtin-macro-redefined",  # To get rid of warnings caused by deterministic build macros (e.g. #define __DATE__ "redacted")
        "-Wno-microsoft-unqualified-friend",  # This shouldn't normally be enabled, but otherwise we get: google/protobuf/map_field.h: warning: unqualified friend declaration referring to type outside of the nearest enclosing namespace is a Microsoft extension; add a nested name specifier (for: friend class DynamicMessage)
    ],
    "//conditions:default": [
    ],
})