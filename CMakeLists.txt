cmake_minimum_required(VERSION 3.0.0)

project(EmoCheck CXX)

set(CMAKE_CXX_STANDARD 17) 
add_definitions(/MT)
add_definitions(/D _UNICODE)
# add_definitions(/utf-8)

# header files
set(hdrs
       emocheck/emocheck.h
)

# source files
set(srcs
        emocheck/emocheck.cpp
        emocheck/main.cpp
)

SOURCE_GROUP("Source Files\\emocheck" FILES ${srcs} )
SOURCE_GROUP("Header Files\\emocheck" FILES ${hdrs} )

add_executable(emocheck ${hdrs} ${srcs})
