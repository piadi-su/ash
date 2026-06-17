cmake_minimum_required(VERSION 3.16)

project(bar C)

add_executable(bar 
	src/main.c
	src/bar.c
)

target_include_directories(bar PRIVATE
    /usr/include/freetype2
    /usr/include/fontconfig
    /usr/include
)

target_compile_options(bar PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)

target_link_libraries(bar PRIVATE
    X11
    Xft
    fontconfig
    freetype
    m
    Xrender
)

