load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository", "new_git_repository")
load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive", "http_file")


def urlsplit(url):
    """ Splits a URL like "https://example.com/a/b?c=d&e#f" into a tuple:
        ("https", ["example", "com"], ["a", "b"], ["c=d", "e"], "f")
    A trailing slash will result in a correspondingly empty final path component.
    """
    split_on_anchor = url.split("#", 1)
    split_on_query = split_on_anchor[0].split("?", 1)
    split_on_scheme = split_on_query[0].split("://", 1)
    if len(split_on_scheme) <= 1:  # Scheme is optional
        split_on_scheme = [None] + split_on_scheme[:1]
    split_on_path = split_on_scheme[1].split("/")
    return {
        "scheme": split_on_scheme[0],
        "netloc": split_on_path[0].split("."),
        "path": split_on_path[1:],
        "query": split_on_query[1].split("&") if len(split_on_query) > 1 else None,
        "fragment": split_on_anchor[1] if len(split_on_anchor) > 1 else None,
    }

def auto_http_archive(*, name=None, url=None, urls=True,
                      build_file=None, build_file_content=None,
                      strip_prefix=True, **kwargs):
    """ Intelligently choose mirrors based on the given URL for the download.
    Either url or urls is required.
    If name         == None , it is auto-deduced, but this is NOT recommended.
    If urls         == True , mirrors are automatically chosen.
    If build_file   == True , it is auto-deduced.
    If strip_prefix == True , it is auto-deduced.
    """
    DOUBLE_SUFFIXES_LOWERCASE = [("tar", "bz2"), ("tar", "gz"), ("tar", "xz")]
    mirror_prefixes = ["https://mirror.bazel.build/"]

    canonical_url = url if url != None else urls[0]
    url_parts = urlsplit(canonical_url)
    url_except_scheme = (canonical_url.replace(url_parts["scheme"] + "://", "")
                         if url_parts["scheme"] != None else canonical_url)
    url_path_parts = url_parts["path"]
    url_filename = url_path_parts[-1]
    url_filename_parts = (url_filename.rsplit(".", 2)
                          if (tuple(url_filename.lower().rsplit(".", 2)[-2:])
                              in DOUBLE_SUFFIXES_LOWERCASE)
                          else url_filename.rsplit(".", 1))
    is_github = url_parts["netloc"] == ["github", "com"]

    if name == None:  # Deduce "com_github_user_project_name" from "https://github.com/user/project-name/..."
        name = "_".join(url_parts["netloc"][::-1] + url_path_parts[:2]).replace("-", "_")

    if build_file == True:
        build_file = "@//thirdparty/%s:%s" % (name, "BUILD." + name)

    if urls == True:
        prefer_url_over_mirrors = is_github
        urls = [mirror_prefix + url_except_scheme
                for mirror_prefix in mirror_prefixes
                if not canonical_url.startswith(mirror_prefix)]
        urls.insert(0 if prefer_url_over_mirrors else len(urls), canonical_url)
    else:
        print("No implicit mirrors used because urls were explicitly provided")

    if strip_prefix == True:
        prefix_without_v = url_filename_parts[0]
        if prefix_without_v.startswith("v") and prefix_without_v[1:2].isdigit():
            # GitHub automatically strips a leading 'v' in version numbers
            prefix_without_v = prefix_without_v[1:]
        strip_prefix = (url_path_parts[1] + "-" + prefix_without_v
                        if is_github and url_path_parts[2:3] == ["archive"]
                        else url_filename_parts[0])

    return http_archive(name=name, url=url, urls=urls, build_file=build_file,
                        build_file_content=build_file_content,
                        strip_prefix=strip_prefix, **kwargs)


def deps_setup():
	auto_http_archive(
        name = "com_github_grpc_grpc",
        # NOTE: If you update this, also update @boringssl's hash.
        url = "https://github.com/grpc/grpc/archive/4790ab6d97e634a1ede983be393f3bb3c132b2f7.tar.gz",
        sha256 = "df83bd8a08975870b8b254c34afbecc94c51a55198e6e3a5aab61d62f40b7274",
    )

    auto_http_archive(
        # This rule is used by @com_github_grpc_grpc, and using a GitHub mirror
        # provides a deterministic archive hash for caching. Explanation here:
        # https://github.com/grpc/grpc/blob/4790ab6d97e634a1ede983be393f3bb3c132b2f7/bazel/grpc_deps.bzl#L102
        name = "boringssl",
        # Ensure this matches the commit used by grpc's bazel/grpc_deps.bzl
        url = "https://github.com/google/boringssl/archive/83da28a68f32023fd3b95a8ae94991a07b1f6c62.tar.gz",
        sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
    )

    auto_http_archive(
        name = "rules_proto_grpc",
        url = "https://github.com/rules-proto-grpc/rules_proto_grpc/archive/a74fef39c5fe636580083545f76d1eab74f6450d.tar.gz",
        sha256 = "2f6606151ec042e23396f07de9e7dcf6ca9a5db1d2b09f0cc93a7fc7f4008d1b",
    )

    git_repository(
    	name = "com_justbuchanan_rules_qt",
    	remote = "https://github.com/justbuchanan/bazel_rules_qt.git",
    	branch = "master",
	)

	new_local_repository(
	    name = "qt",
	    build_file = "@com_justbuchanan_rules_qt//:qt.BUILD",
	    path = "/usr/include/qt",  # arch
	    # path = "/usr/include/x86_64-linux-gnu/qt5",  # debian
	)

	auto_http_archive(
		name = "fmt",
		url = "https://github.com/fmtlib/fmt/archive/7.0.2.tar.gz",
		sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
		build_file = "//thirdparty/jsoncpp:jsoncpp.BUILD",
	)

	auto_http_archive(
		name = "QHotKey",
		url = "https://github.com/Skycoder42/QHotkey/archive/1.4.1.tar.gz",
		sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
		build_file = "//thirdparty/fmt:fmt.BUILD",
	)

	auto_http_archive(
		name = "scope_guard",
		url = "https://github.com/ricab/scope_guard/archive/v0.2.3.tar.gz",
		sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
		build_file = "//thirdparty/scope_guard:scope_guard.BUILD",
	)

	auto_http_archive(
		name = "semver",
		url = "https://github.com/zmarko/semver/archive/1.1.0.tar.gz",
		sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
		build_file = "//thirdparty/semver:semver.BUILD",
	)

	auto_http_archive(
		name = "xz-embedded",
		url = "https://tukaani.org/xz/xz-embedded-20130513.tar.gz",
		sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
		build_file = "//thirdparty/xz-embedded:xz-embedded.BUILD",
	)

	auto_http_archive(
		name = "yaml-cpp",
		url = "https://github.com/jbeder/yaml-cpp/archive/yaml-cpp-0.6.3.tar.gz",
		sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
		build_file = "//thirdparty/yaml-cpp:yaml-cpp.BUILD",
	)

	auto_http_archive(
		name = "jsoncpp",
		url = "https://github.com/open-source-parsers/jsoncpp/archive/1.9.4.tar.gz",
		sha256 = "781fa39693ec2984c71213cd633e9f6589eaaed75e3a9ac413237edec96fd3b9",
		build_file = "//thirdparty/jsoncpp:jsoncpp.BUILD",
	)