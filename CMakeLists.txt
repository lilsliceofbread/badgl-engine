cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 99)

project(badgl)

option(CGLM_SHARED "Shared build" OFF)
option(CGLM_STATIC "Static build" ON)
option(CGLM_USE_C99 "" ON) # C11
option(CGLM_USE_TEST "Enable Tests" OFF) # for make check - make test

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE) # build as static lib
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)

file(GLOB_RECURSE SOURCES CMAKE_CONFIGURE_DEPENDS
     "src/*.c"
     external/glad/src/gl.c)

add_executable(${PROJECT_NAME} ${SOURCES})

# get glfw
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
add_subdirectory(external/cglm)
add_subdirectory(external/assimp)

target_include_directories(${PROJECT_NAME} 
                            PUBLIC include
                            PUBLIC external/stb
                            PUBLIC external/glfw/include 
                            PUBLIC external/glad/include 
                            PUBLIC external/cglm/include
                            PUBLIC external/assimp/include)

target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src PRIVATE external/assimp/lib)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw PUBLIC cglm PUBLIC assimp)