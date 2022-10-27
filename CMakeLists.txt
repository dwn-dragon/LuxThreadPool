cmake_minimum_required(VERSION 3.0.0)

project(LuxThreadPool VERSION 0.2.0.1)
configure_file(config.h.in config.h)

include(CTest)
enable_testing()

set(CXX_STANDARD 20)
set(CXX_STANDARD_REQUIRED ON)

add_subdirectory(src)
add_subdirectory(tests)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
