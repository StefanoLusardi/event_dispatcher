cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

file(STRINGS "VERSION" VERSION_STR)
project(event_dispatcher LANGUAGES CXX VERSION ${VERSION_STR})

include(CMakePrintHelpers)
cmake_print_variables(CMAKE_BUILD_TYPE)
cmake_print_variables(CMAKE_GENERATOR)
cmake_print_variables(CMAKE_CXX_COMPILER_ID)
cmake_print_variables(CMAKE_CXX_COMPILER_VERSION)

set(CMAKE_EXPORT_COMPILE_COMMANDS True)
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/build/modules ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

add_library(event_dispatcher)
add_library(event_dispatcher::event_dispatcher ALIAS event_dispatcher)
set_target_properties(event_dispatcher PROPERTIES LINKER_LANGUAGE CXX) 

target_compile_features(event_dispatcher PUBLIC cxx_std_20)

target_sources(event_dispatcher 
	PUBLIC include/evds/event_dispatcher.hpp
	PUBLIC include/evds/function_traits.hpp
)

target_include_directories(event_dispatcher PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>
	$<INSTALL_INTERFACE:CMAKE_INSTALL_INCLUDEDIR>
)

option(EVDS_ENABLE_UNIT_TESTS "Enable Unit Tests" True)
cmake_print_variables(EVDS_ENABLE_UNIT_TESTS)

option(EVDS_ENABLE_EXAMPLES "Enable Examples" True)
cmake_print_variables(EVDS_ENABLE_EXAMPLES)

option(EVDS_ENABLE_BENCHMARKS "Enable Benchmarks" True)
cmake_print_variables(EVDS_ENABLE_BENCHMARKS)

option(EVDS_ENABLE_WARNINGS_ERROR "Enable Warnings as Errors" True)
cmake_print_variables(EVDS_ENABLE_WARNINGS_ERROR)

option(EVDS_ENABLE_SANITIZER_ADDRESS "Enable Address Sanitizer" True)
cmake_print_variables(EVDS_ENABLE_SANITIZER_ADDRESS)

option(EVDS_ENABLE_SANITIZER_THREAD "Enable Thread Sanitizer" False)
cmake_print_variables(EVDS_ENABLE_SANITIZER_THREAD)

if(EVDS_ENABLE_SANITIZER_ADDRESS AND EVDS_ENABLE_SANITIZER_THREAD)
    message(FATAL_ERROR "It's not possible to set both Address and Thread sanitizers simultaneously.")
endif()

if(EVDS_ENABLE_SANITIZER_ADDRESS)
	include(sanitizers)
    add_sanitizer_address(${PROJECT_NAME})
endif()

if(EVDS_ENABLE_SANITIZER_THREAD)
	include(sanitizers)
    add_sanitizer_thread(${PROJECT_NAME})
endif()

if(EVDS_ENABLE_WARNINGS_ERROR)
	include(warnings)
    add_warnings(${PROJECT_NAME})
    add_warnings_as_errors(${PROJECT_NAME})
endif()

if(EVDS_ENABLE_EXAMPLES)
	include(examples)
	setup_example(${PROJECT_NAME} "examples/example.cpp" "Example")
endif()

if(EVDS_ENABLE_UNIT_TESTS)
	enable_testing()
    include(unit_tests)
	set(UNIT_TESTS_SRC
		tests/src/test_event_dispatcher.cpp
		tests/src/test_event_dispatcher.hpp
		tests/src/main.cpp
	)
	setup_unit_tests(${PROJECT_NAME} "${UNIT_TESTS_SRC}")
endif()

if(EVDS_ENABLE_BENCHMARKS)
	include(benchmarks)
	set(BENCHMARKS_SRC
		benchmarks/src/benchmark_event_dispatcher.cpp
		benchmarks/src/benchmark_event_dispatcher.hpp
		benchmarks/src/main.cpp
	)
	setup_benchmarks(${PROJECT_NAME} "${BENCHMARKS_SRC}")
endif()
