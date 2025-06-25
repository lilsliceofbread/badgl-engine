cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)

project(game)

file(GLOB_RECURSE BADGL_EXAMPLE_SRCS CONFIGURE_DEPENDS "*.c")

if (MSVC)
    set_source_files_properties(${BADGL_EXAMPLE_SRCS} PROPERTIES COMPILE_FLAGS "/W4")
else()
    set_source_files_properties(${BADGL_EXAMPLE_SRCS} PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wconversion -pedantic -Wno-missing-braces")
endif()

add_executable(${PROJECT_NAME} ${BADGL_EXAMPLE_SRCS})

target_include_directories(${PROJECT_NAME} PUBLIC .
                                           PUBLIC ../src/include
                                           PUBLIC external/cimgui
                                           PUBLIC external/cimgui/generator/output)

target_link_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PUBLIC badgl)