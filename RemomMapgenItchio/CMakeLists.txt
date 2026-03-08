cmake_minimum_required(VERSION 4.2)

project(
    RemomMapgenItchio
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

enable_testing()

# SDL2
set(SDL2_BASE "C:/devellib/SDL2-2.32.2")
set(SDL2_DIR "${SDL2_BASE}/cmake" CACHE PATH "" FORCE)
find_package(SDL2 QUIET)
if(NOT SDL2_FOUND)
    # Fallback: manual SDL2 setup
    set(SDL2_INCLUDE_DIRS "${SDL2_BASE}/include")
    if(MSVC)
        set(SDL2_LIBRARIES "${SDL2_BASE}/lib/x64/SDL2.lib" "${SDL2_BASE}/lib/x64/SDL2main.lib")
    else()
        set(SDL2_LIBRARIES "${SDL2_BASE}/lib/x64/SDL2.lib")
    endif()
endif()

# nlohmann/json
include(FetchContent)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# GoogleTest
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
include(GoogleTest)

# Data layer library (no SDL2 dependency)
add_library(MomData STATIC
    src/mom_data/MomWorld.cpp
    src/mom_data/MomEntities.cpp
    src/mom_data/MomGamFile.cpp
    src/scenario/Scenario.cpp
    src/scenario/ScenarioIO.cpp
    src/editor/EditorState.cpp
    src/editor/UndoStack.cpp
)
target_include_directories(MomData PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(MomData PUBLIC nlohmann_json::nlohmann_json)

# Editor executable
add_executable(RemomMapgenItchio WIN32
    src/main.cpp
    src/renderer/Renderer.cpp
    src/renderer/MapRenderer.cpp
    src/renderer/UIRenderer.cpp
    src/ui/ToolPanel.cpp
    src/ui/PropertiesPanel.cpp
    src/ui/Minimap.cpp
)
target_include_directories(RemomMapgenItchio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(RemomMapgenItchio PRIVATE MomData ${SDL2_LIBRARIES})

# Tests
add_executable(RemomTests
    tests/test_MomGamFile.cpp
    tests/test_Scenario.cpp
    tests/test_UndoStack.cpp
)
target_include_directories(RemomTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(RemomTests PRIVATE MomData GTest::gtest_main)
gtest_discover_tests(RemomTests)
