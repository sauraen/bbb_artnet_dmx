set(JUCE_INCLUDE "${CMAKE_CURRENT_SOURCE_DIR}" PARENT_SCOPE)

project(juce)

set(CMAKE_CXX_STANDARD 11) # C++11
set(CMAKE_CXX_STANDARD_REQUIRED ON) # ...is required
set(CMAKE_CXX_EXTENSTIONS OFF) # ...without compiler extensions like gnu++11

set(JUCE_HEADER_FILES juce.h)
set(JUCE_SOURCE_FILES 
    juce_core/juce_core.cpp
)

add_definitions(-DJUCE_GLOBAL_MODULE_SETTINGS_INCLUDED=1 -DJUCE_STANDALONE_APPLICATION=0)
include_directories(.)

add_library(juce STATIC ${JUCE_HEADER_FILES} ${JUCE_SOURCE_FILES})

find_library(PTHREAD_LIB pthread)
find_library(DL_LIB dl)
target_link_libraries(juce PRIVATE "${DL_LIB}" "${PTHREAD_LIB}")
