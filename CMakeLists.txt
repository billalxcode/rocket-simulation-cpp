cmake_minimum_required(VERSION 3.10.0)
project(rocketsim VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(modules/matplotplusplus)

file(GLOB_RECURSE SRC_FILES CONFIGURE_DEPENDS 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

add_executable(rocketsim ${SRC_FILES})
target_link_libraries(rocketsim PRIVATE matplot)