cmake_minimum_required(VERSION 3.23)
project(gitsync VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Third-party includes
set(THIRD_PARTY_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/include)
set(SPDLOG_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/spdlog/include)

# Create library
add_library(gitsync_core STATIC
    src/gitsync.cpp
    src/config_settings.cpp
    src/config_reader.cpp
    src/process_runner.cpp
    src/known_hosts_service.cpp
    src/git_sync_service.cpp)

target_include_directories(gitsync_core PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${THIRD_PARTY_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR})

# Create executable
add_executable(gitsync src/main.cpp)
target_link_libraries(gitsync PRIVATE gitsync_core c++ c++abi pthread)
target_link_options(gitsync PRIVATE -stdlib=libc++)
target_include_directories(gitsync PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${THIRD_PARTY_INCLUDE_DIR}
    ${SPDLOG_INCLUDE_DIR})
target_compile_definitions(gitsync PRIVATE GITSYNC_VERSION="${PROJECT_VERSION}")

# Compiler flags
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(gitsync_core PRIVATE -Wall -Wextra -Wpedantic -stdlib=libc++)
    target_compile_options(gitsync PRIVATE -Wall -Wextra -Wpedantic -stdlib=libc++)
endif()

# Tests
enable_testing()
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/CMakeLists.txt)
    add_subdirectory(tests)
endif()
