# make badgl static library to be linked to

cmake_minimum_required(VERSION 3.13.4)

set(CMAKE_BUILD_TYPE Debug)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE) # build all libs as shared if not explicitly static. DO NOT USE ON WINDOWS.

project(badgl)

set(BADGL_BUILD_EXAMPLE ON)

# assimp build options
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE) # WHY IS THIS ON BY DEFAULT IF COMPILATION FAILS WHEN ITS ON

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

set(IMGUI_STATIC "yes")

file(GLOB_RECURSE BADGL_SRCS CONFIGURE_DEPENDS
     "src/*.c"
     external/glad/src/gl.c)

if (MSVC)
    set_source_files_properties(${BADGL_SRCS} PROPERTIES COMPILE_FLAGS "/W4")
else()
    set_source_files_properties(${BADGL_SRCS} PROPERTIES COMPILE_FLAGS
                                "-Wall -Wextra -Wconversion -pedantic -Wno-missing-braces")
endif()

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

set(cimgui external/cimgui)
set(imgui_impl ${cimgui}/imgui/backends)

file(GLOB 
	CIMGUI_SRCS 
	"${cimgui}/*.cpp" 
	"${cimgui}/imgui/*.cpp"
	"${imgui_impl}/imgui_impl_glfw.cpp"
	"${imgui_impl}/imgui_impl_opengl3.cpp")
add_library(cimgui ${CIMGUI_SRCS})
target_include_directories(cimgui PUBLIC ${cimgui}/imgui PUBLIC external/glfw/include)
target_compile_definitions(cimgui PRIVATE IMGUI_IMPL_API=extern\ \"C\")

# shared lib is faster to build but is broken on windows
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set_target_properties(cimgui PROPERTIES POSITION_INDEPENDENT_CODE ON) # compile cimgui so it can be in a shared library
    add_library(${PROJECT_NAME} SHARED ${BADGL_SRCS})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    add_library(${PROJECT_NAME} ${BADGL_SRCS})
endif()

target_compile_definitions(${PROJECT_NAME} PUBLIC -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_GLFW)
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_API=\ )

target_include_directories(${PROJECT_NAME} 
                            PUBLIC src/include
                            PUBLIC external/stb
                            PUBLIC external/glfw/include 
                            PUBLIC external/glad/include 
                            PUBLIC external/assimp/include
                            PUBLIC external/cimgui
                            PUBLIC external/cimgui/generator/output)

target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src
                                        PRIVATE external/assimp/lib
                                        PRIVATE external/cimgui)

target_link_libraries(${PROJECT_NAME} PUBLIC glfw PUBLIC assimp PUBLIC cimgui)

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(${PROJECT_NAME} PUBLIC GLX) # linux libGLX.so is required for glX extensions (might be lowercase for some)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    target_include_directories(${PROJECT_NAME} PUBLIC external/wgl)
    target_link_libraries(${PROJECT_NAME} PUBLIC OpenGL32) # windows needs OpenGL32.lib
endif()

if(NOT MSVC)
    target_link_libraries(${PROJECT_NAME} PUBLIC m) # libm is libc math library, not automatically linked for some reason
endif()

if(MINGW)
	target_link_options(${PROJECT_NAME} PRIVATE "-mconsole") # prevent mingw opening a second console
endif()

if(BADGL_BUILD_EXAMPLE)
    add_subdirectory(example)
endif()