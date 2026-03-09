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

# SDL2 — Emscripten provides SDL2 via its ports system (-s USE_SDL=2),
# so we only do manual SDL2 setup on native builds.
if(NOT EMSCRIPTEN)
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
else()
    # Emscripten: SDL2 is provided as a port; clear variables so
    # target_link_libraries references don't break.
    set(SDL2_INCLUDE_DIRS "")
    set(SDL2_LIBRARIES "")
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

# Data layer library (no SDL2 dependency, no platform APIs per Constitution Principle IV)
add_library(MomData STATIC
    src/mom_data/MomWorld.cpp
    src/mom_data/MomEntities.cpp
    src/mom_data/MomGamFile.cpp
    src/scenario/Scenario.cpp
    src/scenario/ScenarioIO.cpp
    src/editor/EditorState.cpp
    src/editor/UndoStack.cpp
    src/mapgen/SimplexNoise.cpp
    src/mapgen/MapGenerator.cpp
    src/validation/SmoothingValidator.cpp
)
target_include_directories(MomData PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(MomData PUBLIC nlohmann_json::nlohmann_json)

# Editor executable (platform-specific code lives here, NOT in MomData)
# Emscripten builds omit WIN32 — the subsystem flag is Windows-only.
if(EMSCRIPTEN)
    add_executable(RemomMapgenItchio
        src/main.cpp
        src/renderer/Renderer.cpp
        src/renderer/MapRenderer.cpp
        src/renderer/UIRenderer.cpp
        src/ui/ToolPanel.cpp
        src/ui/PropertiesPanel.cpp
        src/ui/GameOptionsPanel.cpp
        src/ui/WizardPanel.cpp
        src/ui/Minimap.cpp
        src/platform/FileDialog.cpp
    )
else()
    add_executable(RemomMapgenItchio WIN32
        src/main.cpp
        src/renderer/Renderer.cpp
        src/renderer/MapRenderer.cpp
        src/renderer/UIRenderer.cpp
        src/ui/ToolPanel.cpp
        src/ui/PropertiesPanel.cpp
        src/ui/GameOptionsPanel.cpp
        src/ui/WizardPanel.cpp
        src/ui/Minimap.cpp
        src/platform/FileDialog.cpp
    )
endif()
target_include_directories(RemomMapgenItchio PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SDL2_INCLUDE_DIRS}
)
target_link_libraries(RemomMapgenItchio PRIVATE MomData ${SDL2_LIBRARIES})

# Emscripten-specific compile and linker flags. Powered by Claude.
if(EMSCRIPTEN)
    # USE_SDL=2 must be a compile option too so Emscripten provides
    # the real SDL2 headers instead of the fakesdl error stubs.
    target_compile_options(RemomMapgenItchio PRIVATE -sUSE_SDL=2)
    target_link_options(RemomMapgenItchio PRIVATE
        -sUSE_SDL=2
        -sALLOW_MEMORY_GROWTH=1
        -sEXPORTED_FUNCTIONS=['_main','_mom_file_upload_callback','_malloc','_free']
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
    )

    # Use custom shell template if it exists (created in T037).
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/shell.html")
        target_link_options(RemomMapgenItchio PRIVATE
            --shell-file ${CMAKE_CURRENT_SOURCE_DIR}/src/shell.html
        )
    endif()

    # Produce .html output so Emscripten generates the HTML harness.
    set_target_properties(RemomMapgenItchio PROPERTIES SUFFIX ".html")

    # Install target: copies only deployment files to a clean directory.
    # itch.io requires index.html, so rename the HTML output.
    # Usage: cmake --install build_wasm --prefix build_wasm/dist
    # Powered by Claude.
    install(FILES ${CMAKE_BINARY_DIR}/RemomMapgenItchio.html DESTINATION . RENAME index.html)
    install(FILES ${CMAKE_BINARY_DIR}/RemomMapgenItchio.js DESTINATION .)
    install(FILES ${CMAKE_BINARY_DIR}/RemomMapgenItchio.wasm DESTINATION .)
endif()

# Tests
add_executable(RemomTests
    tests/test_MomGamFile.cpp
    tests/test_Scenario.cpp
    tests/test_UndoStack.cpp
    tests/test_MapGenerator.cpp
    tests/test_SmoothingValidator.cpp
)
target_include_directories(RemomTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(RemomTests PRIVATE MomData GTest::gtest_main)
gtest_discover_tests(RemomTests)
