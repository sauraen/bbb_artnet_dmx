cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(bbb_artnet_dmx LANGUAGES CXX)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(Juce)
link_libraries(juce)
include_directories(${JUCE_INCLUDE})

set(HEADER_FILES )
set(SOURCE_FILES
    src/main.cpp)
    
add_executable(ArtnetDMX ${HEADER_FILES} ${SOURCE_FILES})

