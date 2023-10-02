cmake_minimum_required(VERSION 3.26)
project(main VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


find_package(Boost 1.70 REQUIRED COMPONENTS program_options)
include_directories(${Boost_INCLUDE_DIRS})

set(BUILD_SHARED_LIBS FALSE)

add_subdirectory(sfml)
include_directories(sfml/include)
include_directories(include)


set(SOURCES
    main.cpp
    src/PathMonitor.cpp
    src/Configuration.cpp
    src/AppModel.cpp
    src/AppCLIView.cpp
    src/AppCLIController.cpp
    src/AppCLIFeatures.cpp
    src/TelnetClient.cpp
    src/Utils.cpp
)

add_executable(main ${SOURCES})
target_link_libraries(main PRIVATE sfml-network sfml-main Boost::program_options)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    include(CTest)
    add_subdirectory(tests)
endif()