cmake_minimum_required(VERSION 3.5.0)
project(MoX-CombatGrid VERSION 0.1.0 LANGUAGES C CXX)

# Link this "library" to use the warnings specified in CompilerWarnings.cmake.
add_library(project_warnings INTERFACE)
include(cmake/CompilerWarnings.cmake)
set_project_warnings(project_warnings)

list(APPEND CMAKE_PREFIX_PATH "C:\\devellib\\SDL2-2.30.9\\cmake")

find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIR})

include_directories("imgui-docking")
include_directories("imgui-docking/backends")
include_directories("stb")

set(SOURCES
    main.cpp
)

set(MOX_DIR ../ReMoM/src/MoX)
set(MOX_SOURCES
${MOX_DIR}/Allocate.C
${MOX_DIR}/Allocate.H
${MOX_DIR}/Exit.C
${MOX_DIR}/Exit.H
${MOX_DIR}/FLIC_Draw.C
${MOX_DIR}/FLIC_Draw.H
${MOX_DIR}/LBX_Load.C
${MOX_DIR}/LBX_Load.H
${MOX_DIR}/MoX_BASE.H
${MOX_DIR}/MoX_BITS.H
${MOX_DIR}/MoX_DBG.C
${MOX_DIR}/MoX_DBG.H
${MOX_DIR}/MoX_TYPE.H
)

set(REMOM_DIR ../ReMoM/src)

set(IMGUI_DIR imgui-docking)
set(IMGUI_SOURCES
    ${IMGUI_DIR}/imconfig.h
    ${IMGUI_DIR}/imgui.cpp
    ${IMGUI_DIR}/imgui.h
    ${IMGUI_DIR}/imgui_demo.cpp
    ${IMGUI_DIR}/imgui_draw.cpp
    ${IMGUI_DIR}/imgui_internal.h
    ${IMGUI_DIR}/imgui_tables.cpp
    ${IMGUI_DIR}/imgui_widgets.cpp
    ${IMGUI_DIR}/imstb_rectpack.h
    ${IMGUI_DIR}/imstb_textedit.h
    ${IMGUI_DIR}/imstb_truetype.h
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdl2.h
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
    ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.h
)

add_library(MoX STATIC ${MOX_SOURCES})

add_executable(${PROJECT_NAME} ${SOURCES} ${IMGUI_SOURCES})

target_link_libraries(${PROJECT_NAME} SDL2::SDL2 SDL2::SDL2main)

target_link_libraries(${PROJECT_NAME} MoX)
include_directories(${MOX_DIR})

add_custom_command(
    TARGET MoX-CombatGrid POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_if_different
         "C:\\devellib\\SDL2-2.30.9\\lib\\x64\\SDL2.dll"
         $<TARGET_FILE_DIR:MoX-CombatGrid>
)

add_custom_command(
    TARGET MoX-CombatGrid POST_BUILD
     COMMAND ${CMAKE_COMMAND} -E copy_if_different
         "C:\\STU\\devel\\STU-Extras\\MoX-CombatGrid\\MyImage01.jpg"
         $<TARGET_FILE_DIR:MoX-CombatGrid>
)
