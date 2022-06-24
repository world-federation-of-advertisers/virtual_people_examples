workspace(name = "virtual_people_examples")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Common-cpp
http_archive(
    name = "wfa_common_cpp",
    sha256 = "e8efc0c9f5950aff13a59f21f40ccc31c26fe40c800743f824f92df3a05588b2",
    strip_prefix = "common-cpp-0.5.0",
    url = "https://github.com/world-federation-of-advertisers/common-cpp/archive/refs/tags/v0.5.0.tar.gz",
)

load("@wfa_common_cpp//build:common_cpp_repositories.bzl", "common_cpp_repositories")

common_cpp_repositories()

load("@wfa_common_cpp//build:common_cpp_deps.bzl", "common_cpp_deps")

common_cpp_deps()

# Virtual-people-common
http_archive(
    name = "virtual_people_common",
    sha256 = "2929e1d83594a294f86bf46dab84d312456a9bae71d5abc191f4094580a0727e",
    strip_prefix = "virtual-people-common-1e601efd5ad61385626d8bc3739adbb3b8e68c70",
    url = "https://github.com/world-federation-of-advertisers/virtual-people-common/archive/1e601efd5ad61385626d8bc3739adbb3b8e68c70.tar.gz",
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

# Virtual-people-core-serving
git_repository(
    name = "virtual_people_core_serving",
    commit = "5043e4658cfddadc47aa46f9fe555b364a2fbda1",
    remote = "https://github.com/world-federation-of-advertisers/virtual-people-core-serving",
)
