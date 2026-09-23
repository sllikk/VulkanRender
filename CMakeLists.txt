cmake_minimum_required(VERSION 4.1)

project(Engine)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(PROJECT_SRC
    "src/engine.h"
    "src/engine.cpp"
    "src/VkBootstrap.h"
    "src/VkBootstrap.cpp"
    "src/VkBootstrapDispatch.h"
    "src/utils.hpp"
    "src/FrameResources.h"
    "src/FrameResources.cpp"
    "src/pipeline_manager.h"
    "src/pipeline_manager.cpp"
)

find_package(Vulkan REQUIRED)

include(FetchContent)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.5.1
)

FetchContent_Declare(
        glm
        GIT_REPOSITORY	https://github.com/g-truc/glm.git
        GIT_TAG 	0af55ccecd98d4e5a8d1fad7de25ba429d60e863 #refs/tags/1.0.1
)

FetchContent_Declare(
        fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt.git
        GIT_TAG		   12.0.0
)


FetchContent_MakeAvailable(glfw glm fmt)

add_subdirectory(third-party/vk-bootstrap)

add_executable(${PROJECT_NAME} ${PROJECT_SRC})

set(GLSLC "${Vulkan_INCLUDE_DIR}/../Bin/glslc.exe")

message("CMAKE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}")

set(SHADERS_IN_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADERS_OUT_DIR "${CMAKE_BINARY_DIR}/compiled_shaders")

file(GLOB SHADERS "${SHADERS_IN_DIR}/*.vert" "${SHADERS_IN_DIR}/*.frag")

file(MAKE_DIRECTORY ${SHADERS_OUT_DIR})

foreach(SHADER ${SHADERS})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SHADER_OUT_NAME "${SHADERS_OUT_DIR}/${SHADER_NAME}.spv")
    list(APPEND SHADER_OUT_NAMES ${SHADER_OUT_NAME})
    add_custom_command(
            MAIN_DEPENDENCY ${SHADER}
            OUTPUT ${SHADER_OUT_NAME}
            COMMAND ${GLSLC}  ${SHADER} "-o" ${SHADER_OUT_NAME}
            VERBATIM)
endforeach()

add_custom_target(build_shaders DEPENDS ${SHADER_OUT_NAMES})

add_dependencies(${PROJECT_NAME} build_shaders)

target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDEB_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm::glm fmt::fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
