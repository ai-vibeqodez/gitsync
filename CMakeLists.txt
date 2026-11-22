cmake_minimum_required(VERSION 3.23)
project(gitsync VERSION 0.1.0 LANGUAGES CXX)

# Options
option(GITSYNC_BUILD_TESTS "Build tests" ON)
option(GITSYNC_WARNINGS_AS_ERRORS "Treat warnings as errors" OFF)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# FetchContent for dependencies
include(FetchContent)

# nlohmann_json
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS NAMES nlohmann_json
)

# spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.14.1
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS NAMES spdlog
)

set(FETCHCONTENT_QUIET OFF)
set(FETCHCONTENT_TIMEOUT 180)
FetchContent_MakeAvailable(nlohmann_json spdlog)

# Create library
add_library(gitsync_core STATIC
    src/gitsync.cpp
    src/config_settings.cpp
    src/config_reader.cpp
    src/process_runner.cpp
    src/known_hosts_service.cpp
    src/git_sync_service.cpp)

target_include_directories(gitsync_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(gitsync_core PUBLIC
    nlohmann_json::nlohmann_json
    spdlog::spdlog)

# Create executable
add_executable(gitsync src/main.cpp)
target_link_libraries(gitsync PRIVATE gitsync_core)
target_compile_definitions(gitsync PRIVATE GITSYNC_VERSION="${PROJECT_VERSION}")

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(gitsync_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(gitsync PRIVATE -Wall -Wextra -Wpedantic)
    if(GITSYNC_WARNINGS_AS_ERRORS)
        target_compile_options(gitsync_core PRIVATE -Werror)
        target_compile_options(gitsync PRIVATE -Werror)
    endif()
endif()

# Tests
enable_testing()
if(GITSYNC_BUILD_TESTS AND EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()
