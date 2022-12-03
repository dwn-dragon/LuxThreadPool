cmake_minimum_required(VERSION 3.0.0)
project(LuxThreadPool VERSION 0.2.1.0)

set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

add_library(
	LuxThreadPool STATIC EXCLUDE_FROM_ALL
	${CMAKE_CURRENT_SOURCE_DIR}/lux_thread_pool.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/lux_thread_pool.cpp
)
target_include_directories(
	LuxThreadPool PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
set_target_properties(
	LuxThreadPool PROPERTIES
	CXX_STANDARD 20
	CXX_STANDARD_REQUIRED ON
)

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
	target_link_libraries(LuxThreadPool PRIVATE atomic)
endif()

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	include(CTest)
	enable_testing()

	add_subdirectory(tests)
endif()

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
