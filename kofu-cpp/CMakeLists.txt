# =============================================================================
# CMakeLists.txt — Kofu C++ Backend
# =============================================================================
#
# Build:
#   mkdir build && cd build
#   cmake .. -DCMAKE_BUILD_TYPE=Release
#   cmake --build . -j$(nproc)
#
# Package:
#   cpack                           # Generates .deb, .tar.gz, etc.
#   cpack -G NSIS                   # Windows installer (cross-compile)
#   cpack -G DragNDrop              # macOS .dmg
#
# =============================================================================

cmake_minimum_required(VERSION 3.20)

project(kofu
    VERSION      2.0.0
    DESCRIPTION  "Kofu — Local AI Document Assistant (C++ Backend)"
    HOMEPAGE_URL "https://github.com/shebxyd-pixel/Proyecto_Kofu"
    LANGUAGES    CXX
)

# ---------------------------------------------------------------------------
# C++ Standard
# ---------------------------------------------------------------------------
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Use C++20 if the compiler supports it (for std::format, etc.)
include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++20" COMPILER_SUPPORTS_CXX20)
if(COMPILER_SUPPORTS_CXX20)
    set(CMAKE_CXX_STANDARD 20)
    message(STATUS "Using C++20")
else()
    message(STATUS "Using C++17 (C++20 not supported by compiler)")
endif()

# ---------------------------------------------------------------------------
# Build type defaults
# ---------------------------------------------------------------------------
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# ---------------------------------------------------------------------------
# Compiler flags — optimized for i5 8th Gen (Coffee Lake)
# ---------------------------------------------------------------------------
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Release: aggressive optimization for the target CPU
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG -march=native -flto")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "-flto -s")

    # Debug: full debug info + sanitizers
    set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fsanitize=address,undefined -fno-omit-frame-pointer")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG "-fsanitize=address,undefined")

    # Common warnings (always on)
    add_compile_options(
        -Wall -Wextra -Wpedantic
        -Wno-unused-parameter      # Many handler signatures have unused params
        -Wshadow
        -Wconversion
        -Wnon-virtual-dtor
    )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(CMAKE_CXX_FLAGS_RELEASE "/O2 /DNDEBUG /GL")
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")
    add_compile_options(/W4 /permissive-)
endif()

# ---------------------------------------------------------------------------
# Thread configuration — 6 worker threads (leaving 2 for OS + Ollama)
# ---------------------------------------------------------------------------
set(KOFU_THREAD_POOL_SIZE 6 CACHE STRING "HTTP server thread pool size")
add_compile_definitions(KOFU_THREAD_POOL_SIZE=${KOFU_THREAD_POOL_SIZE})

# ---------------------------------------------------------------------------
# Fetch header-only dependencies (cpp-httplib, nlohmann/json, pugixml)
# ---------------------------------------------------------------------------
include(cmake/FetchDependencies.cmake)

# ---------------------------------------------------------------------------
# Find system libraries
# ---------------------------------------------------------------------------
# libzip — required for .docx/.pptx ZIP archive creation
if(WIN32 OR MSVC)
    find_package(libzip CONFIG REQUIRED)
    add_library(PkgConfig::LIBZIP ALIAS libzip::zip)
    message(STATUS "libzip: found via vcpkg")
else()
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBZIP REQUIRED IMPORTED_TARGET libzip)
    message(STATUS "libzip: ${LIBZIP_VERSION} (${LIBZIP_LINK_LIBRARIES})")
endif()

# Threads — required by cpp-httplib
find_package(Threads REQUIRED)

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
set(KOFU_SOURCES
    src/main.cpp
    src/config.cpp
    src/json_utils.cpp
    src/sanitizer.cpp
    src/ollama_client.cpp
    src/model_router.cpp
    src/knowledge_base.cpp
    src/reasoning.cpp
    src/docx_generator.cpp
    src/pptx_generator.cpp
    src/web_research.cpp
    src/assistant.cpp
    src/http_server.cpp
)

set(KOFU_HEADERS
    include/kofu/config.hpp
    include/kofu/json_utils.hpp
    include/kofu/sanitizer.hpp
    include/kofu/ollama_client.hpp
    include/kofu/model_router.hpp
    include/kofu/knowledge_base.hpp
    include/kofu/reasoning.hpp
    include/kofu/docx_generator.hpp
    include/kofu/pptx_generator.hpp
    include/kofu/web_research.hpp
    include/kofu/assistant.hpp
    include/kofu/http_server.hpp
)

# ---------------------------------------------------------------------------
# Main executable
# ---------------------------------------------------------------------------
add_executable(kofu ${KOFU_SOURCES} ${KOFU_HEADERS})

target_include_directories(kofu PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(kofu PRIVATE
    httplib::httplib             # HTTP server + client
    nlohmann_json::nlohmann_json # JSON parsing
    pugixml::pugixml             # XML DOM manipulation
    PkgConfig::LIBZIP            # ZIP archive read/write
    Threads::Threads             # pthreads
)

# ---------------------------------------------------------------------------
# Install rules
# ---------------------------------------------------------------------------
include(GNUInstallDirs)

install(TARGETS kofu
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Install web frontend assets
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../web/")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../web/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/kofu/web
        FILES_MATCHING
            PATTERN "*.html"
            PATTERN "*.js"
            PATTERN "*.css"
            PATTERN "*.ico"
    )
endif()

# Install document templates
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../templates/")
    install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../templates/
        DESTINATION ${CMAKE_INSTALL_DATADIR}/kofu/templates
    )
endif()

# Install .env config template
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.env")
    install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/../.env
        DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/kofu
        RENAME kofu.env.example
    )
endif()

# Install desktop entry (Linux)
if(UNIX AND NOT APPLE)
    install(FILES packaging/kofu.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()

# ---------------------------------------------------------------------------
# CPack — Cross-platform packaging
# ---------------------------------------------------------------------------
set(CPACK_PACKAGE_NAME "kofu")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "Kofu Project")
set(CPACK_PACKAGE_CONTACT "kofu@project.dev")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "Kofu")

# -- Debian .deb --
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libzip4 (>= 1.7)")
set(CPACK_DEBIAN_PACKAGE_SECTION "misc")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Kofu Maintainer <kofu@project.dev>")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "${PROJECT_HOMEPAGE_URL}")

# -- RPM --
set(CPACK_RPM_PACKAGE_REQUIRES "libzip >= 1.7")
set(CPACK_RPM_PACKAGE_LICENSE "Custom")

# -- NSIS (Windows) --
set(CPACK_NSIS_DISPLAY_NAME "Kofu AI Assistant")
set(CPACK_NSIS_PACKAGE_NAME "Kofu")
set(CPACK_NSIS_ENABLE_UNINSTALL_BEFORE_INSTALL ON)

# -- macOS DragNDrop --
set(CPACK_DMG_VOLUME_NAME "Kofu")

# Generator selection based on platform
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;TGZ")
elseif(UNIX)
    set(CPACK_GENERATOR "DEB;TGZ")
endif()

# Static tar.gz is always available
list(APPEND CPACK_GENERATOR "TGZ")
list(REMOVE_DUPLICATES CPACK_GENERATOR)

include(CPack)

# ---------------------------------------------------------------------------
# Summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== Kofu Build Configuration ===")
message(STATUS "  Version     : ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type  : ${CMAKE_BUILD_TYPE}")
message(STATUS "  Compiler    : ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "  Thread Pool : ${KOFU_THREAD_POOL_SIZE} threads")
message(STATUS "  Generators  : ${CPACK_GENERATOR}")
message(STATUS "================================")
message(STATUS "")
