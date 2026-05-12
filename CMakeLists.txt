cmake_minimum_required(VERSION 3.10)

project(PicoGS1 VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_compile_options(-fsanitize=address)
    add_link_options(-fsanitize=address)
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-constant-logical-operand")

find_package(SDL2 REQUIRED)

find_path(PORTMIDI_INCLUDE_DIR portmidi.h)
find_library(PORTMIDI_LIBRARY portmidi)

if (NOT PORTMIDI_INCLUDE_DIR OR NOT PORTMIDI_LIBRARY)
    message(FATAL_ERROR "portmidi library not found")
endif()

include_directories(${SDL2_INCLUDE_DIRS} ${PORTMIDI_INCLUDE_DIR})

add_subdirectory(emulator)
add_subdirectory(editor)

add_executable(gs1_standalone test/standalone.cpp)

target_link_libraries(gs1_standalone ${SDL2_LIBRARIES} ${PORTMIDI_LIBRARY} gs1emu)

target_include_directories(gs1_standalone PRIVATE test)