cmake_minimum_required(VERSION 3.14)

project(GLES1_Wrapper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(
        GLES1_Wrapper SHARED
        GLES1_Wrapper.cpp
)

target_link_libraries(
        GLES1_Wrapper
        Qt${QT_VERSION_MAJOR}::Core
        Qt${QT_VERSION_MAJOR}::Gui
        Qt${QT_VERSION_MAJOR}::OpenGL
)
