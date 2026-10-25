cmake_minimum_required(VERSION 3.16)
project(PluginCalculator CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/EHsc)
endif()


set(OUT_DIR "${CMAKE_BINARY_DIR}/bin$<0:>")

add_executable(calc src/main.cpp src/calculator.cpp src/plugin_loader.cpp)
target_include_directories(calc PRIVATE include)
set_target_properties(calc PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}")

foreach(plugin funcsin funcdeg funcln funccos)
    add_library(${plugin} SHARED plugins/${plugin}.cpp)
    target_include_directories(${plugin} PRIVATE include)
    set_target_properties(${plugin} PROPERTIES PREFIX "" RUNTIME_OUTPUT_DIRECTORY "${OUT_DIR}/plugins")
endforeach()