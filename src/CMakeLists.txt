# https://jonathanhamberg.com/post/cmake-embedding-git-hash/

add_executable(gdf
 main.cpp 
)

include(FetchContent)
FetchContent_Declare(
    argparse
    GIT_REPOSITORY https://github.com/p-ranav/argparse.git
)
FetchContent_MakeAvailable(argparse)

include_directories(/usr/include)
link_directories(/usr/lib)

target_link_libraries(gdf argparse git2)

add_custom_command(TARGET gdf POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_BINARY_DIR}/compile_commands.json
        ${CMAKE_SOURCE_DIR}/compile_commands.json
    COMMENT "Add clangd to the src directory")

