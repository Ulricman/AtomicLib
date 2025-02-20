cmake_minimum_required(VERSION 3.28.3)
project(AtomicLib LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

file(GLOB Sources CONFIGURE_DEPENDS "*.cpp")

foreach(Source ${Sources})
    get_filename_component(ExecName ${Source} NAME_WE)
    add_executable(${ExecName} ${Source})
    target_include_directories(${ExecName} PUBLIC "${CMAKE_SOURCE_DIR}/include")
endforeach()