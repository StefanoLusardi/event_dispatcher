cmake_minimum_required(VERSION 3.10 FATAL_ERROR)

function(add_example EXAMPLE_TARGET)
	add_executable(${EXAMPLE_TARGET} src/${EXAMPLE_TARGET}.cpp)
	target_link_libraries(${EXAMPLE_TARGET} PRIVATE evds::evds)
	target_compile_features(${EXAMPLE_TARGET} PRIVATE cxx_std_20)
endfunction()

# add_example(event_loop)
# add_example(simple_dispatch)
add_example(bus)