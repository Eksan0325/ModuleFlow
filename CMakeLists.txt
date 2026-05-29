cmake_minimum_required(VERSION 3.10)

# Version template "master.middle.patch"
project(mflow VERSION 0.1.0 LANGUAGES CXX) 

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS NO)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -frtti -Wno-register")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3 -fno-omit-frame-pointer")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(FATAL_ERROR "This project only supports Linux. Current system: ${CMAKE_SYSTEM_NAME}")
endif()

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type (Debug, Release, RelWithDebInfo)" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release RelWithDebInfo)
endif()

message(STATUS "Building for architecture: ${CMAKE_SYSTEM_PROCESSOR}")

file(GLOB_RECURSE SRC_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c"
)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/3rdparty/lib)

set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")


add_library(mflow_obj OBJECT ${SRC_FILES})

target_include_directories(mflow_obj PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

target_include_directories(mflow_obj PUBLIC
    "$<BUILD_INTERFACE:${INCLUDE_DIR}>"   
    "$<INSTALL_INTERFACE:include>"        
)

set_target_properties(mflow_obj PROPERTIES
    POSITION_INDEPENDENT_CODE ON
)


add_library(mflow_shared SHARED $<TARGET_OBJECTS:mflow_obj>)


set_target_properties(mflow_shared PROPERTIES
    OUTPUT_NAME mflow
    VERSION     ${PROJECT_VERSION} 
    SOVERSION   ${PROJECT_VERSION_MAJOR}
)

target_include_directories(mflow_shared PUBLIC
    "$<BUILD_INTERFACE:${INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:include>"
)


add_library(mflow_static STATIC $<TARGET_OBJECTS:mflow_obj>)

set_target_properties(mflow_static PROPERTIES
    OUTPUT_NAME mflow_static       
)

target_include_directories(mflow_static PUBLIC
    "$<BUILD_INTERFACE:${INCLUDE_DIR}>"
    "$<INSTALL_INTERFACE:include>"
)

install(TARGETS mflow_shared mflow_static
    LIBRARY DESTINATION lib      
    ARCHIVE DESTINATION lib      
)


install(DIRECTORY ${INCLUDE_DIR}/
    DESTINATION include
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp"
        PATTERN "*.inl"
)

message(STATUS "----------------------------------------------------------------")
message(STATUS "Project: ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "C++ Standard: C++${CMAKE_CXX_STANDARD}")
message(STATUS "Library output:")
message(STATUS "  Shared: libmflow.so.${PROJECT_VERSION} (SONAME libmflow.so.${PROJECT_VERSION_MAJOR})")
message(STATUS "  Static: libmflow_static.a")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "----------------------------------------------------------------")