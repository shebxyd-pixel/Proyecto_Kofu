# =============================================================================
# FetchDependencies.cmake — Kofu C++ Backend
# =============================================================================
# Pulls header-only dependencies via FetchContent at configure time.
# System libraries (libzip) are found via find_package in the root CMakeLists.
# =============================================================================

include(FetchContent)

# ---------------------------------------------------------------------------
# cpp-httplib — Single-header HTTP server + client
# https://github.com/yhirose/cpp-httplib
# ---------------------------------------------------------------------------
FetchContent_Declare(
    httplib
    GIT_REPOSITORY https://github.com/yhirose/cpp-httplib.git
    GIT_TAG        v0.18.3
    GIT_SHALLOW    TRUE
)
# cpp-httplib provides a CMake target 'httplib::httplib' when built via CMake.
# We set HTTPLIB_COMPILE to build as a compiled library (faster recompilation)
# rather than header-only (which would recompile the ~12K line header every time).
set(HTTPLIB_COMPILE  ON  CACHE BOOL "Compile cpp-httplib as a library" FORCE)
set(HTTPLIB_REQUIRE_OPENSSL OFF CACHE BOOL "Don't require OpenSSL" FORCE)
set(HTTPLIB_REQUIRE_ZLIB    OFF CACHE BOOL "Don't require zlib" FORCE)
set(HTTPLIB_INSTALL  OFF CACHE BOOL "Skip install targets" FORCE)

FetchContent_MakeAvailable(httplib)

# ---------------------------------------------------------------------------
# nlohmann/json — JSON for Modern C++
# https://github.com/nlohmann/json
# ---------------------------------------------------------------------------
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
    GIT_SHALLOW    TRUE
)
set(JSON_BuildTests    OFF CACHE BOOL "Disable json tests" FORCE)
set(JSON_Install       OFF CACHE BOOL "Skip json install" FORCE)
set(JSON_MultipleHeaders OFF CACHE BOOL "Use single header" FORCE)

FetchContent_MakeAvailable(nlohmann_json)

# ---------------------------------------------------------------------------
# pugixml — Light-weight XML processing library
# https://github.com/zeux/pugixml
# ---------------------------------------------------------------------------
FetchContent_Declare(
    pugixml
    GIT_REPOSITORY https://github.com/zeux/pugixml.git
    GIT_TAG        v1.14
    GIT_SHALLOW    TRUE
)
set(PUGIXML_BUILD_TESTS OFF CACHE BOOL "Disable pugixml tests" FORCE)

FetchContent_MakeAvailable(pugixml)

# ---------------------------------------------------------------------------
# Provide summary of fetched dependencies
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== Kofu Dependencies ===")
message(STATUS "  cpp-httplib : v0.18.3 (compiled library)")
message(STATUS "  nlohmann/json : v3.11.3")
message(STATUS "  pugixml     : v1.14")
message(STATUS "=========================")
message(STATUS "")
