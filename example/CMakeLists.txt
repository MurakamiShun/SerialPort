cmake_minimum_required(VERSION 3.9)

project(serial_test LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 11)
find_package(SerialPort)
add_executable(${PROJECT_NAME} test.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE SerialPort)
