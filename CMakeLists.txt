# make badgl static library to be linked to

cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)

project(badgl)

# build the example game thing
set(BADGL_BUILD_EXAMPLE ON)

# assimp build options
set(BUILD_SHARED_LIBS ON CACHE INTERNAL "" FORCE) # build as SHARED lib, so compilation is faster
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE) # WHY IS THIS ON BY DEFAULT IF COMPILATION FAILS WHEN ITS ON

file(GLOB_RECURSE BADGL_SRCS
     "src/*.c"
     external/glad/src/gl.c)

if (MSVC) # don't know if this works; i don't use msvc
    set_source_files_properties(${BADGL_SRCS} PROPERTIES COMPILE_FLAGS "/W4")
else()
    set_source_files_properties(${BADGL_SRCS} PROPERTIES COMPILE_FLAGS "-Wall -Wextra -Wconversion -pedantic -Wno-missing-braces")
endif()

add_library(${PROJECT_NAME} ${BADGL_SRCS})

# get git package if exists
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    # update submodules if necessary
    option(GIT_SUBMODULE "Check submodules during build" ON)
    if(GIT_SUBMODULE)
        message(STATUS "Updating submodules")
        execute_process(
            COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            RESULT_VARIABLE GIT_SUBMOD_RESULT
        )
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "Git submodule update failed with ${GIT_SUBMOD_RESULT}")
        endif()
    endif()
endif()

add_subdirectory(external/glfw)
add_subdirectory(external/assimp)
if(BADGL_BUILD_EXAMPLE)
    add_subdirectory(example)
endif(BADGL_BUILD_EXAMPLE)

target_include_directories(${PROJECT_NAME} 
                            PUBLIC src/include
                            PUBLIC external/stb
                            PUBLIC external/glfw/include 
                            PUBLIC external/glad/include 
                            PUBLIC external/assimp/include)

target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src PRIVATE external/assimp/lib)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw PUBLIC assimp PUBLIC m)