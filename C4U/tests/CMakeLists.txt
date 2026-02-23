
include(GoogleTest)

add_executable(
    func_extract_parse_test
    func_extract_parse_test.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/../C4U/FuncExtract.c
)

target_include_directories(func_extract_parse_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/../C4U)
target_link_libraries(func_extract_parse_test GTest::gtest_main)

gtest_discover_tests(func_extract_parse_test)
