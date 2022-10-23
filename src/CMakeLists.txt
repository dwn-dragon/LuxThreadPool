#	CMake /	src
#	

add_executable(Demo)

target_include_directories(
	Demo 
	PRIVATE ${CMAKE_SOURCE_DIR}
	PRIVATE ${CMAKE_BINARY_DIR}
)
target_sources(
	Demo 
	PRIVATE main.cpp
	PRIVATE lux_thread_pool.cpp
)
