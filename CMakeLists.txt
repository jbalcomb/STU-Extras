# rm -rf build/*
# cmake -S . -B build       defaults to "Visual Studio 17 2022", MSBuild, MSVC
# cmake --build build --config Release      defaults to Debug configuration
# cmake --install build --config Release    defaults to Release configuration
# cpack -G ZIP -C Release                   WTF? -C is Configuration, --config is Config File
# cpack -G NSIS -C Release
# C:/devel/EduMat/Learning_CMake/me_cmake_040/build/ProjectToot--win64.exe

cmake_minimum_required(VERSION 3.22)

project(
    ProjectToot
    VERSION 0.0.1
    LANGUAGES CXX
)

enable_testing()
# GoogleTest requires at least C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

configure_file(ProjectTootConfig.h.in ProjectToot.h)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(TootLib)

add_executable(TootMe Toot.cpp)

target_link_libraries(TootMe PRIVATE TootLib)
target_include_directories(TootMe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/TootLib "${PROJECT_BINARY_DIR}")

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

install(TARGETS TootMe)

include(InstallRequiredSystemLibraries)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/License.txt")
set(CPACK_PACKAGE_VERSION_MAJOR "${ProjectToot_VERSION_MAJOR}")
set(CPACK_PACKAGE_VERSION_MINOR "${ProjectToot_VERSION_MINOR}")
set(CPACK_GENERATOR "TGZ")
set(CPACK_SOURCE_GENERATOR "TGZ")
include(CPack)
