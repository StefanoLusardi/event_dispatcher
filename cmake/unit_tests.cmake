function(setup_unit_tests TARGET_NAME TARGET_SRC_UNIT_TEST)	
	set(TARGET_NAME_UNIT_TEST ${TARGET_NAME}_UnitTests)
	find_package(GTest REQUIRED)
	include(GoogleTest)
	add_executable(${TARGET_NAME_UNIT_TEST})
	target_sources(${TARGET_NAME_UNIT_TEST} PRIVATE ${TARGET_SRC_UNIT_TEST})
	target_compile_features(${TARGET_NAME_UNIT_TEST} PUBLIC cxx_std_20)
	target_link_libraries(${TARGET_NAME_UNIT_TEST} PRIVATE GTest::GTest PRIVATE ${TARGET_NAME})
	gtest_discover_tests(${TARGET_NAME_UNIT_TEST})
endfunction()