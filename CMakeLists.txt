cmake_minimum_required(VERSION 3.9)
project(mat267ti)

include(${CE_TOOLCHAIN_DIR}/cmake/toolchain.cmake OPTIONAL)

add_executable(mat267ti
    src/main.c
    src/eval.c
    src/compute.c
    src/ui.c
    src/fields.c
)

target_compile_options(mat267ti PRIVATE -Wall -Os)
target_link_libraries(mat267ti ce)
