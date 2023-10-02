find_package(GTest CONFIG REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

add_executable(configuration ConfigurationTest.cpp FileTestHelper.hpp ../src/Configuration.cpp)
target_link_libraries(configuration GTest::gtest GTest::gtest_main)
add_test(NAME UNIT_TESTS_CONFIGURATION COMMAND configuration)
