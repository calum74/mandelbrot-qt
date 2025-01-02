cmake_minimum_required(VERSION 3.21)

project(mandelbrot-qt VERSION 1.0 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

enable_testing()
add_subdirectory(mandelbrot)
add_subdirectory(src)

