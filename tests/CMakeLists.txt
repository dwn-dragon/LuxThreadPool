#	CMake / tests
#

set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

configure_file(config.h.in config.h)
link_libraries(LuxThreadPool)

add_executable(Demo)
add_executable(test.cstr)
add_executable(test.submit)
add_executable(test.run)

set_target_properties(
	Demo test.cstr test.run test.submit
	PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

target_include_directories( Demo PRIVATE ${CMAKE_CURRENT_BINARY_DIR} )

target_sources(Demo PRIVATE demo.cpp)
target_sources(test.cstr	PRIVATE test.cstr.cpp)
target_sources(test.submit	PRIVATE test.submit.cpp)
target_sources(test.run		PRIVATE test.run.cpp)

add_test(NAME Construction COMMAND test.cstr)
add_test(NAME Submit COMMAND test.submit)

add_test(NAME Run.100 COMMAND test.run 100)
add_test(NAME Run.10000 COMMAND test.run 10000)
