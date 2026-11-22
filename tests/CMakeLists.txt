add_executable(gitsync_tests gitsync_tests.cpp)

target_link_libraries(gitsync_tests PRIVATE gitsync_core)

target_compile_options(gitsync_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME gitsync_unit_tests COMMAND gitsync_tests)
