cmake_minimum_required(VERSION 3.0.0)

project(LuxThreadPool VERSION 0.1.0)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

add_subdirectory(src)
add_subdirectory(tests)

set_property(TARGET LuxThreadPool PROPERTY CXX_STANDARD 20)
set_property(TARGET LuxThreadPool PROPERTY CXX_STANDARD_REQUIRED ON)
