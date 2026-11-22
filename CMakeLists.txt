cmake_minimum_required(VERSION 3.23)

include(FetchContent)
FetchContent_Declare(
    aw_cmake_modules
    GIT_REPOSITORY https://github.com/ai-vibeqodez/template_cpp
    GIT_TAG main
    GIT_SHALLOW ON)
FetchContent_MakeAvailable(aw_cmake_modules)
list(APPEND CMAKE_MODULE_PATH "${aw_cmake_modules_SOURCE_DIR}/cmake")

include(AWProject)
aw_project_prepare(NAME gitsync NAMESPACE aw::)
project(${AW_PROJECT_NAME} VERSION ${AW_PROJECT_VERSION} LANGUAGES CXX)
aw_project_setup(PREFIX GITSYNC BUILD_TESTS)
aw_check_cxx23_runtime()

# Fetch dependencies first
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
    GIT_SHALLOW TRUE
    FIND_PACKAGE_ARGS NAMES nlohmann_json
)

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

# Create core library with dependencies linked
include(Library/Target)
aw_add_library(TARGET gitsync_core TYPE STATIC ALIAS aw::gitsync_core
    SOURCES
        src/gitsync.cpp
        src/config_settings.cpp
        src/config_reader.cpp
        src/process_runner.cpp
        src/known_hosts_service.cpp
        src/git_sync_service.cpp
    PUBLIC_INCLUDE_DIRS include
    PUBLIC_LINKS
        nlohmann_json::nlohmann_json
        spdlog::spdlog)

# Create executable
aw_add_executable(TARGET gitsync SOURCES src/main.cpp
    LINK_LIBRARIES aw::gitsync_core)

target_compile_definitions(gitsync PRIVATE GITSYNC_VERSION="${PROJECT_VERSION}")

if(GITSYNC_WARNINGS_AS_ERRORS)
    aw_configure_compiler(TARGET gitsync STRICT_WARNINGS)
endif()

aw_setup_tests(OPTION GITSYNC_BUILD_TESTS DIRECTORY tests)
