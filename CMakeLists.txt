cmake_minimum_required(VERSION 3.26)
project(main VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

#strongly recommend use of vcpkg to install these libraries if missing
find_package(CURL CONFIG REQUIRED)
include_directories(${CURL_INCLUDE_DIRS})
find_package(Boost 1.70 REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory(ftp)
add_subdirectory(telnet)
include_directories(ftp/FTP)
include_directories(telnet/include)
include_directories(include)


set(SOURCES
    main.cpp
    src/PathMonitor.cpp
)

add_executable(main ${SOURCES})
target_link_libraries(main PRIVATE CURL::libcurl ftpclient)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()