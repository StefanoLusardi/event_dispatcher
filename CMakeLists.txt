cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

file(STRINGS "VERSION" VERSION_STR)
project(dispatcher 
	LANGUAGES CXX
	VERSION ${VERSION_STR}
)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_library(evds INTERFACE)
add_library(evds::evds ALIAS evds)

target_compile_features(evds INTERFACE cxx_std_17)
target_include_directories(evds INTERFACE
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:CMAKE_INSTALL_INCLUDEDIR>)

option(EVDS_ENABLE_EXAMPLES "Enable Examples" True)

if(EVDS_ENABLE_EXAMPLES)
	add_subdirectory(examples)
endif()