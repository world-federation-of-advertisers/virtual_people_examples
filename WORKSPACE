load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Common-cpp
http_archive(
    name = "wfa_common_cpp",
    sha256 = "a964e147d4516ec40560531ad5fd654bd4aef37fc05c58d44ad1749b2dc466dc",
    strip_prefix = "common-cpp-0.6.0",
    url = "https://github.com/world-federation-of-advertisers/common-cpp/archive/refs/tags/v0.6.0.tar.gz",
)

load("@wfa_common_cpp//build:common_cpp_repositories.bzl", "common_cpp_repositories")

common_cpp_repositories()

load("@wfa_common_cpp//build:common_cpp_deps.bzl", "common_cpp_deps")

common_cpp_deps()

# Virtual-people-common
http_archive(
    name = "virtual_people_common",
    sha256 = "23591a296092da0cca03732072a84625ead0eec36ef8f38a87940e6af2d706a5",
    strip_prefix = "virtual-people-common-ed17b00b7fe7a2c5646479e42dc3bb32b2f5c80f",
    url = "https://github.com/world-federation-of-advertisers/virtual-people-common/archive/ed17b00b7fe7a2c5646479e42dc3bb32b2f5c80f.tar.gz",
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# Virtual-people-core-serving
git_repository(
    name = "virtual_people_core_serving",
    commit = "086be7da4d08cd0d0cab1c25eb6835d269abfe7b",
    remote = "https://github.com/world-federation-of-advertisers/virtual-people-core-serving",
)
