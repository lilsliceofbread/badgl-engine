cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)

project(learnopengl)

file(
    GLOB SOURCES 
    src/*.c
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

target_include_directories(${PROJECT_NAME} PUBLIC inc PUBLIC external/glfw/include PUBLIC external/glad/include)

target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src)

target_link_libraries(${PROJECT_NAME} glfw)