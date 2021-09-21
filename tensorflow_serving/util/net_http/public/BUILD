# Description: Library containing public files shared by client and server sides.

package(
    default_visibility = [
        ":shared_files",
        "//tensorflow_serving:internal",
        "//tensorflow_serving/util/net_http:__subpackages__",
    ],
)

package_group(name = "shared_files")

licenses(["notice"])  # Apache 2.0

cc_library(
    name = "shared_files",
    srcs = [
        "header_names.cc",
    ],
    hdrs = [
        "header_names.h",
        "response_code_enum.h",
    ],
)