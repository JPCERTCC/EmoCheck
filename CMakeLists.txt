
cmake_minimum_required(VERSION 3.15)

# Use statically-linked runtime library in Release build.
# https://cmake.org/cmake/help/git-stage/variable/CMAKE_MSVC_RUNTIME_LIBRARY.html
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
else()
    cmake_policy(SET CMP0091 NEW)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded")
    add_definitions(-DDEBUG)
endif()

# check compile target
if(COMPILE_TARGET STREQUAL "32")
    message(STATUS "Compile target: 32bit binary")
else()
    set(COMPILE_TARGET "64")
    message(STATUS "Compile target: 64bit binary")
endif()

# project name and target language
project(emocheck CXX)

# compile options for windows binary
add_definitions(-std=c++17)
add_definitions(-DUNICODE)
add_definitions(-D_CONSOLE)
add_definitions(-D_MBCS) ## support multibyte character set 
add_definitions(-D_WINDOWS)
add_definitions(-D_WIN32)
add_definitions(-Wall) ## warning all
add_definitions(-Wextra)
add_definitions(-Wno-missing-field-initializers)
add_definitions(-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH)

# header files
set(utils_hdrs
        emocheck/utils/utils.hpp
        emocheck/utils/native.hpp
        emocheck/utils/file.hpp
        emocheck/utils/registry.hpp
        emocheck/utils/process.hpp
        emocheck/utils/service.hpp
)
set(modules_hdrs
        emocheck/modules/scan.hpp
)
set(main_hdrs
    emocheck/emocheck.hpp
)
# source files
set(utils_srcs
        emocheck/utils/utils.cpp
        emocheck/utils/native.cpp
        emocheck/utils/file.cpp
        emocheck/utils/registry.cpp
        emocheck/utils/process.cpp
        emocheck/utils/service.cpp
)
set(modules_srcs
        emocheck/modules/scan_v1.cpp
        emocheck/modules/scan_v2.cpp
        emocheck/modules/scan_v3.cpp
        emocheck/modules/scan_v4.cpp
        emocheck/modules/scan_v5.cpp
)
set(main_srcs
        emocheck/emocheck.cpp
        emocheck/main.cpp
)

set(hdrs
         ${main_hdrs}
         ${utils_hdrs}
         ${modules_hdrs}
)

set(srcs
          ${main_srcs}
          ${utils_srcs}
          ${modules_srcs}
)

SOURCE_GROUP("Source Files\\emocheck" FILES ${srcs} )
SOURCE_GROUP("Header Files\\emocheck" FILES ${hdrs} )

add_executable(emocheck ${hdrs} ${srcs})

# switch compile target to 32bit
if(COMPILE_TARGET STREQUAL "32")
    set_target_properties(emocheck PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()
