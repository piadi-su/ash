cmake_minimum_required(VERSION 3.16)

project(pi-bar C)

add_executable(pi-bar 
	src/main.c
)

target_compile_options(pi-bar PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(pi-bar PRIVATE
	X11
)

