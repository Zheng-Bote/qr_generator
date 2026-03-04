cmake_minimum_required(VERSION 3.24)

# cmake -S . -B build
# cmake --build build -j$(nproc)

# ./qrgen "Hello World" bunt.svg 10 FF5500 222222


project(QRGenerator
VERSION 0.2.0
DESCRIPTION "QR Code Generator (CLI & Header-only library)"
HOMEPAGE_URL "https://github.com/Zheng-Bote/qr_generator"
LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Configure (für rz_config.hpp) ---
set(EXECUTABLE_NAME "${PROJECT_NAME}")
set(PROG_LONGNAME "Native C++ Dependency Scanner & SBOM Generator")
set(PROG_CREATED "2026")
set(PROG_AUTHOR "ZHENG Bote")
set(PROG_ORGANIZATION_NAME "ZHENG Robert")
set(PROG_ORGANIZATION_DOMAIN "net.hase-zheng")
# create rz_config.hpp
# add_subdirectory(configure)

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


# ==============================================================================
# Bibliothek (Library Target) - perfect for external consumers
# ==============================================================================
add_library(qr_generator INTERFACE)
add_library(qr_generator::qr_generator ALIAS qr_generator)

# Damit fremde Projekte den "include"-Ordner und die Header von qrencode/webp finden
target_include_directories(qr_generator INTERFACE 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${qrencode_SOURCE_DIR}>
    $<BUILD_INTERFACE:${webp_SOURCE_DIR}>
)

# Abhängigkeiten weitervererben
target_link_libraries(qr_generator INTERFACE qrencode webp)


# ==============================================================================
# Executable (CLI Tool) - built only if this project is the top-level project
# ==============================================================================
if(PROJECT_IS_TOP_LEVEL)
    add_executable(qrgen src/main.cpp)
    
    # Links now cleanly against the own Interface-Target
    target_link_libraries(qrgen PRIVATE qr_generator)
endif()


# libs.txt Generate (timing-safe via GENERATE)
file(GENERATE OUTPUT "${CMAKE_BINARY_DIR}/libs.txt" 
     CONTENT "$<TARGET_PROPERTY:qr_generator,LINK_LIBRARIES>")

# ==============================================================================
# Install & Packaging (macOS .pkg)
# ==============================================================================
if(PROJECT_IS_TOP_LEVEL)
    install(TARGETS qrgen
        DESTINATION bin
        COMPONENT Application
    )

    set(CPACK_GENERATOR "productbuild")
    set(CPACK_PACKAGE_NAME "qrgen")
    set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
    set(CPACK_PACKAGE_VENDOR "${PROG_ORGANIZATION_NAME}")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr/local")
    
    # Required for macOS installer to not have disabled Continue button
    set(CPACK_COMPONENTS_ALL Application)
    set(CPACK_COMPONENT_APPLICATION_DISPLAY_NAME "QR Generator Application")
    set(CPACK_COMPONENT_APPLICATION_DESCRIPTION "The CLI tool qrgen")
    
    include(CPack)
endif()
