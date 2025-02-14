cmake_minimum_required(VERSION 3.15)
project(CppVue VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuration Emscripten
set(CMAKE_TOOLCHAIN_FILE "$ENV{EMSDK}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake")

# Options de compilation WebAssembly
set(CMAKE_EXECUTABLE_SUFFIX ".js")
set(EMSCRIPTEN_GENERATE_BITCODE_STATIC_LIBRARIES ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s WASM=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s EXPORTED_FUNCTIONS=['_main']")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s NO_EXIT_RUNTIME=1")

# Options de compilation
option(BUILD_TESTS "Build tests" OFF)

# Sources principales
file(GLOB_RECURSE LIB_SOURCES
    "src/core/*.cpp"
    "src/core/*.hpp"
    "src/wasm/*.cpp"
    "src/wasm/*.hpp"
    "src/compiler/*.cpp"
    "src/compiler/*.hpp"
)

# Bibliothèque cppvue
add_library(cppvue STATIC ${LIB_SOURCES})
target_include_directories(cppvue PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Exemples
add_subdirectory(examples/todo)
