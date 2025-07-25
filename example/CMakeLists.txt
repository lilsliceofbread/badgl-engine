cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)

project(game)

file(GLOB_RECURSE BADGL_EXAMPLE_SRCS CONFIGURE_DEPENDS "*.c")

add_executable(${PROJECT_NAME} ${BADGL_EXAMPLE_SRCS})

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wconversion -Wpedantic -Wno-cast-function-type -Wno-missing-braces)
endif()

target_include_directories(${PROJECT_NAME} 
    PRIVATE .
    PRIVATE ../src/include
    PRIVATE external/cimgui
    PRIVATE external/cimgui/generator/output)

target_link_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PRIVATE badgl)
