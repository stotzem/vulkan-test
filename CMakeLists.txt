cmake_minimum_required(VERSION 3.24)

project(vulkan-test VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SDL2 REQUIRED)
find_package(Vulkan REQUIRED)

add_executable(game
    src/Application.cpp
)

target_include_directories(game PRIVATE
    src
    ${Vulkan_INCLUDE_DIRS}
)

target_link_libraries(game PRIVATE
    SDL2::SDL2
    ${Vulkan_LIBRARIES}
)
