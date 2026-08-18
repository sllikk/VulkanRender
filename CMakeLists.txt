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
target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDEB_DIRS})
target_link_libraries(${PROJECT_NAME} PUBLIC Vulkan::Vulkan)
target_link_libraries(${PROJECT_NAME} PUBLIC glfw glm::glm fmt::fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC vk-bootstrap::vk-bootstrap)
