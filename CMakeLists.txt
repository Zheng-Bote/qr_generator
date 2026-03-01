cmake_minimum_required(VERSION 3.24)

# cmake -S . -B build
# cmake --build build -j$(nproc)

# ./qrgen "Hello World" bunt.svg 10 FF5500 222222

project(QRGenerator
VERSION 0.1.0
DESCRIPTION "QR Code Generator (CLI & Header-only library)"
HOMEPAGE_URL "https://github.com/ZHeng-Bote/qr_generator"
LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# --- libqrencode ---
set(WITH_TOOLS OFF CACHE BOOL "Disable tools build" FORCE)
set(WITH_TESTS OFF CACHE BOOL "Disable tests build" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static lib" FORCE)

FetchContent_Declare(
    qrencode
    GIT_REPOSITORY https://github.com/fukuchi/libqrencode.git
    GIT_TAG        v4.1.1
)
FetchContent_MakeAvailable(qrencode)

# --- libwebp ---
set(WEBP_BUILD_ANIM_UTILS OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_CWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_DWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_GIF2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_IMG2WEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_VWEBP OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPINFO OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_WEBPMUX OFF CACHE BOOL "" FORCE)
set(WEBP_BUILD_EXTRAS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static lib" FORCE)

FetchContent_Declare(
    webp
    GIT_REPOSITORY https://github.com/webmproject/libwebp.git
    GIT_TAG        v1.3.2
)
FetchContent_MakeAvailable(webp)

# --- Main ---
add_executable(qrgen src/main.cpp)

# Tell the compiler where our header files are
target_include_directories(qrgen PRIVATE 
    include
    ${qrencode_SOURCE_DIR}
    ${webp_SOURCE_DIR}
)

target_link_libraries(qrgen PRIVATE qrencode webp)