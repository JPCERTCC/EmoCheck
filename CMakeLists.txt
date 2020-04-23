cmake_minimum_required(VERSION 3.0.0)

project(EmoCheck CXX)

set(CMAKE_CXX_STANDARD 17) 
add_definitions(/MT)
add_definitions(/D _UNICODE)
# add_definitions(/utf-8)

# header files
set(utils_hdrs
        emocheck/utils/utils.hpp
)

set(modules_hdrs
        emocheck/modules/scan_v1.hpp
        emocheck/modules/scan_v2.hpp
)

set(hdrs
       emocheck/emocheck.hpp
       ${utils_hdrs}
       ${modules_hdrs}
)

# source files
set(utils_srcs
        emocheck/utils/utils.cpp
)

set(modules_srcs
        emocheck/modules/scan_v1.cpp
        emocheck/modules/scan_v2.cpp
)

set(srcs
        emocheck/emocheck.cpp
        emocheck/main.cpp
        ${utils_srcs}
        ${modules_srcs}
)

SOURCE_GROUP("Source Files\\emocheck" FILES ${srcs} )
SOURCE_GROUP("Header Files\\emocheck" FILES ${hdrs} )

add_executable(emocheck ${hdrs} ${srcs})
