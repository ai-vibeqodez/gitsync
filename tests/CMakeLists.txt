add_executable(gitsync_tests gitsync_tests.cpp)

target_link_libraries(gitsync_tests PRIVATE
    gitsync_core
    c++
    c++abi
    pthread)

target_link_options(gitsync_tests PRIVATE -stdlib=libc++)

target_include_directories(gitsync_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../third_party/spdlog/include)

target_compile_options(gitsync_tests PRIVATE -Wall -Wextra -Wpedantic -stdlib=libc++)

add_test(NAME gitsync_unit_tests COMMAND gitsync_tests)
