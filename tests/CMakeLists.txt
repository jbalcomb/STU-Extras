
add_executable(hello_test hello_test.cpp)

include(GoogleTest)

target_link_libraries(hello_test GTest::gtest_main)

gtest_discover_tests(hello_test)
