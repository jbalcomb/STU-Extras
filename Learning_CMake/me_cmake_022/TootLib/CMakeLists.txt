
add_library(TootLib STATIC TootLib.c)

# target_include_directories(TootLib PRIVATE "${PROJECT_SOURCE_DIR}")
target_include_directories(TootLib PRIVATE ${PROJECT_SOURCE_DIR}/TootLib)
# target_include_directories(TootLib PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# target_link_libraries(TootLib )

add_subdirectory(tests)
