include(FetchContent)

find_package(GTest CONFIG QUIET)

if(NOT GTest_FOUND)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.17.0.zip)
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()

add_executable(commonlib_tests)
file(GLOB TEST_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
target_sources(commonlib_tests PRIVATE ${TEST_SOURCES})
target_link_libraries(commonlib_tests PRIVATE CommonLib GTest::gtest_main)
set_target_properties(commonlib_tests PROPERTIES CXX_SCAN_FOR_MODULES ON)

include(GoogleTest)
gtest_discover_tests(commonlib_tests)
