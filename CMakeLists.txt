cmake_minimum_required(VERSION 3.13.4)
project(badgl)
set(CMAKE_BUILD_TYPE Debug)
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "" FORCE)

#
# BADGL BUILD OPTIONS
#

set(BADGL_BUILD_EXAMPLE ON)



file(GLOB_RECURSE BADGL_SRCS CONFIGURE_DEPENDS "src/*.c")
add_library(${PROJECT_NAME} ${BADGL_SRCS}) # TODO: move glad somewhere else here

set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD 23)
set_property(TARGET ${PROJECT_NAME} PROPERTY C_STANDARD_REQUIRED ON)

target_include_directories(${PROJECT_NAME} 
                            PUBLIC src/include
                            PUBLIC shaders/include)

# set warning options
if(MSVC)
    set_source_files_properties(${BADGL_SRCS} PROPERTIES COMPILE_FLAGS "/W4")
else()
    set_source_files_properties(${BADGL_SRCS} PROPERTIES COMPILE_FLAGS
                                "-Wall -Wextra -Wconversion -Wpedantic -Wno-cast-function-type -Wno-missing-braces")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    add_compile_definitions(BGL_NO_DEBUG)
endif()

#
# external modules
#

# update git submodules if necessary
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
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

# external includes
target_include_directories(${PROJECT_NAME} 
                            PUBLIC external/stb
                            PUBLIC external/glfw/include 
                            PUBLIC external/glad/include 
                            PUBLIC external/assimp/include
                            PUBLIC external/cimgui
                            PUBLIC external/cimgui/generator/output)

# assimp
set(ASSIMP_NO_EXPORT ON CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE INTERNAL "" FORCE)
set(ASSIMP_WARNINGS_AS_ERRORS OFF CACHE INTERNAL "" FORCE)
add_subdirectory(external/assimp)

# glfw
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(external/glfw)

# glad
add_library(glad "external/glad/src/glad.c")
target_include_directories(glad PUBLIC "external/glad/include")

# cimgui
set(IMGUI_STATIC "yes")
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
target_compile_definitions(${PROJECT_NAME} PUBLIC -DCIMGUI_USE_OPENGL3 -DCIMGUI_USE_GLFW)
target_compile_definitions(${PROJECT_NAME} PRIVATE IMGUI_IMPL_API=\ )

# link all static libraries to badgl
target_link_directories(${PROJECT_NAME} PRIVATE external/glfw/src
                                        PRIVATE external/assimp/lib
                                        PRIVATE external/cimgui)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw PUBLIC assimp PUBLIC cimgui PUBLIC glad)



# platform specific stuff

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
