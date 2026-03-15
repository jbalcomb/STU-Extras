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

# SDL2_ttf — font rendering for UI text labels.
# Native: manual setup from local install. Emscripten: port system.
# Powered by Claude.
if(NOT EMSCRIPTEN)
    set(SDL2_TTF_BASE "C:/devellib/SDL2_ttf-2.22.0")
    set(SDL2_TTF_INCLUDE_DIRS "${SDL2_TTF_BASE}/include")
    set(SDL2_TTF_LIBRARIES "${SDL2_TTF_BASE}/lib/x64/SDL2_ttf.lib")
else()
    set(SDL2_TTF_INCLUDE_DIRS "")
    set(SDL2_TTF_LIBRARIES "")
endif()

# nlohmann/json
include(FetchContent)
set(JSON_Install OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# GoogleTest — disable install so cmake --install only includes our files.
# Powered by Claude.
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
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
    ${SDL2_TTF_INCLUDE_DIRS}
)
target_link_libraries(RemomMapgenItchio PRIVATE MomData ${SDL2_LIBRARIES} ${SDL2_TTF_LIBRARIES})

# Emscripten-specific compile and linker flags. Powered by Claude.
if(EMSCRIPTEN)
    # USE_SDL=2 and USE_SDL_TTF=2 must be compile options too so Emscripten
    # provides the real headers instead of the fakesdl error stubs.
    # Powered by Claude.
    target_compile_options(RemomMapgenItchio PRIVATE -sUSE_SDL=2 -sUSE_SDL_TTF=2)
    target_link_options(RemomMapgenItchio PRIVATE
        -sUSE_SDL=2
        -sUSE_SDL_TTF=2
        -sALLOW_MEMORY_GROWTH=1
        -sSTACK_SIZE=262144
        -sEXPORTED_FUNCTIONS=['_main','_mom_file_upload_callback','_malloc','_free']
        "-sEXPORTED_RUNTIME_METHODS=['ccall','cwrap']"
        --embed-file ${CMAKE_CURRENT_SOURCE_DIR}/assets/Cousine-Regular.ttf@/assets/Cousine-Regular.ttf
    )

    # Debug build: DWARF debug info, source maps, and runtime assertions.
    # Powered by Claude.
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_options(RemomMapgenItchio PRIVATE -g -gsource-map)
        target_link_options(RemomMapgenItchio PRIVATE
            -g
            -gsource-map
            -sASSERTIONS=2
            -sSTACK_OVERFLOW_CHECK=2
            -sSAFE_HEAP=1
        )
        target_compile_options(MomData PRIVATE -g -gsource-map)
    endif()

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
    install(FILES ${CMAKE_BINARY_DIR}/RemomMapgenItchio.html DESTINATION . RENAME index.html COMPONENT deploy)
    install(FILES ${CMAKE_BINARY_DIR}/RemomMapgenItchio.js DESTINATION . COMPONENT deploy)
    install(FILES ${CMAKE_BINARY_DIR}/RemomMapgenItchio.wasm DESTINATION . COMPONENT deploy)
endif()

# Copy SDL2_ttf.dll to build output for native builds.
# Powered by Claude.
if(NOT EMSCRIPTEN)
    add_custom_command(TARGET RemomMapgenItchio POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_TTF_BASE}/lib/x64/SDL2_ttf.dll"
            $<TARGET_FILE_DIR:RemomMapgenItchio>
    )
endif()

# Tests
add_executable(RemomTests
    tests/test_MomGamFile.cpp
    tests/test_Scenario.cpp
    tests/test_UndoStack.cpp
    tests/test_MapGenerator.cpp
    tests/test_SmoothingValidator.cpp
    tests/test_WizardPresets.cpp
)
target_include_directories(RemomTests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
target_link_libraries(RemomTests PRIVATE MomData GTest::gtest_main)
gtest_discover_tests(RemomTests)
