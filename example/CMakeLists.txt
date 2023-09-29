cmake_minimum_required(VERSION 3.13.4)

project(game)

if (MSVC)
    add_compile_options(/W4)
else()
    add_compile_options(-Wall -Wextra -Wconversion -pedantic)
endif()

file(GLOB_RECURSE BADGL_EXAMPLE_SRCS CMAKE_CONFIGURE_DEPENDS 
    "*.c")

add_executable(${PROJECT_NAME} ${BADGL_EXAMPLE_SRCS})

target_include_directories(${PROJECT_NAME} 
                            PUBLIC ../include
                            PUBLIC ../external/stb
                            PUBLIC ../external/glfw/include 
                            PUBLIC ../external/glad/include 
                            PUBLIC ../external/assimp/include
)

target_link_directories(${PROJECT_NAME} PRIVATE src)

target_link_libraries(${PROJECT_NAME} PUBLIC badgl)