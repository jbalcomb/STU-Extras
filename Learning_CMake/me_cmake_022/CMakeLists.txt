# rm -rf build/*
# cmake -S . -B build
# cmake --build build
# cmake --build build --config Debug
# ctest --test-dir build
# ctest --test-dir build --rerun-failed --output-on-failure
# ctest -R 'Foo.' -j4 --output-on-failure

cmake_minimum_required(VERSION 3.22)

project(ProjectToot LANGUAGES CXX)

enable_testing()

# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(TootLib)

add_executable(TootMe Toot.cpp)

target_include_directories(TootMe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/TootLib)
# target_include_directories(TootMe PRIVATE ${CMAKE_SOURCE_DIR}/TootLib)
# target_include_directories(TootLib PRIVATE ${PROJECT_SOURCE_DIR}/TootLib)

target_link_libraries(TootMe PRIVATE TootLib)

include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

add_subdirectory(tests)
