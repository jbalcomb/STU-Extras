project(FuncExtract LANGUAGES C)

add_executable(FuncExtract main.c FuncExtract.c)

target_include_directories(FuncExtract PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
