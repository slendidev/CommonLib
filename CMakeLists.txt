cmake_minimum_required(VERSION 4.1)

project(
  CommonLib
  VERSION 0.1.0
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
  add_compile_options(-Wall -Wextra -Wunused -Wconversion)
endif()

include(CTest)

add_subdirectory(src)

if(PROJECT_IS_TOP_LEVEL)
  add_executable(demo demo.cpp)
  target_link_libraries(demo PRIVATE CommonLib)
  set_target_properties(demo PROPERTIES CXX_SCAN_FOR_MODULES ON)

  if(BUILD_TESTING)
    add_subdirectory(tests)
  endif()
endif()
