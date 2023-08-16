find_package(GTest CONFIG REQUIRED)
include_directories(${GTest_INCLUDE_DIRS})

add_executable(pathmonitor PathMonitorTest.cpp FileTestHelper.hpp ../src/PathMonitor.cpp)
target_link_libraries(pathmonitor GTest::gtest GTest::gtest_main)
add_test(NAME UNIT_TESTS_PATH_MONITOR COMMAND pathmonitor)

add_executable(configuration ConfigurationTest.cpp FileTestHelper.hpp ../src/Configuration.cpp)
target_link_libraries(configuration GTest::gtest GTest::gtest_main)
add_test(NAME UNIT_TESTS_CONFIGURATION COMMAND configuration)
