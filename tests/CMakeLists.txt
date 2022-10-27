#	CMake / tests
#

set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

add_executable(test.cstr)
add_executable(test.submit)
add_executable(test.run)

set_target_properties(
	test.cstr test.run test.submit
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

target_include_directories(test.cstr PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(test.cstr PRIVATE test.cstr.cpp PRIVATE lux_thread_pool.cpp)
target_include_directories(test.submit PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(test.submit PRIVATE test.submit.cpp PRIVATE lux_thread_pool.cpp)
target_include_directories(test.run PRIVATE ${CMAKE_SOURCE_DIR})
target_sources(test.run PRIVATE test.run.cpp PRIVATE lux_thread_pool.cpp)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_link_libraries(test.cstr PRIVATE atomic)
	target_link_libraries(test.submit PRIVATE atomic)
	target_link_libraries(test.run PRIVATE atomic)
endif()

add_test(NAME Construction COMMAND test.cstr)
add_test(NAME Submit COMMAND test.submit)

add_test(NAME Run.100 COMMAND test.run 100)
add_test(NAME Run.10000 COMMAND test.run 10000)
