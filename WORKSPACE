load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

# Common-cpp
http_archive(
    name = "wfa_common_cpp",
    sha256 = "e0e1f5eed832ef396109354a64c6c1306bf0fb5ea0b449ce6ee1e8edc6fe279d",
    strip_prefix = "common-cpp-43c75acc3394e19bcfd2cfe8e8e2454365d26d60",
    url = "https://github.com/world-federation-of-advertisers/common-cpp/archive/43c75acc3394e19bcfd2cfe8e8e2454365d26d60.tar.gz",
)

load("@wfa_common_cpp//build:deps.bzl", "common_cpp_deps")

common_cpp_deps()

# Protobuf
http_archive(
    name = "com_google_protobuf",
    sha256 = "355cf346e6988fd219ff7b18e6e68a742aaef09a400a0cf2860e7841468a12ac",
    strip_prefix = "protobuf-3.15.7",
    urls = ["https://github.com/protocolbuffers/protobuf/releases/download/v3.15.7/protobuf-all-3.15.7.tar.gz"],
)

load("@com_google_protobuf//:protobuf_deps.bzl", "protobuf_deps")

protobuf_deps()

# Glog
http_archive(
    name = "com_github_gflags_gflags",
    sha256 = "34af2f15cf7367513b352bdcd2493ab14ce43692d2dcd9dfc499492966c64dcf",
    strip_prefix = "gflags-2.2.2",
    urls = ["https://github.com/gflags/gflags/archive/v2.2.2.tar.gz"],
)

http_archive(
    name = "com_github_google_glog",
    sha256 = "21bc744fb7f2fa701ee8db339ded7dce4f975d0d55837a97be7d46e8382dea5a",
    strip_prefix = "glog-0.5.0",
    urls = ["https://github.com/google/glog/archive/v0.5.0.zip"],
)

load("@bazel_tools//tools/build_defs/repo:git.bzl", "git_repository")

git_repository(
    name = "virtual_people_common",
    remote = "https://github.com/world-federation-of-advertisers/virtual-people-common",
    commit = "2fe66f15018200668d30b9ddedd6dd8e54bc8982",
)

git_repository(
    name = "virtual_people_core_serving",
    remote = "https://github.com/world-federation-of-advertisers/virtual-people-core-serving",
    commit = "5301f8bf04d4eda85a881b5c7bcf081adaedf923",
    repo_mapping = {"@com_google_farmhash": "@farmhash"}
)

# TODO(@tcsnfkx): Remove this after references to cross_media_measurement in
#                 virtual_people_core_serving are moved to wfa_common_cpp.
git_repository(
    name = "cross_media_measurement",
    remote = "https://github.com/world-federation-of-advertisers/cross-media-measurement",
    commit = "a4863588aa84c965e6ec0d0b1d6e535b0d86d388",
)
