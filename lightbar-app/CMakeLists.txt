cmake_minimum_required(VERSION 3.30)
project(lightbar_app)

set(CMAKE_CXX_STANDARD 20)

add_executable(lightbar_app main.cpp)

target_link_libraries(lightbar_app d3d11 dxgi)
