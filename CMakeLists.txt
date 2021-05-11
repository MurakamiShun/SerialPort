cmake_minimum_required(VERSION 3.9)
message(STATUS "CMAKE_ROOT: " ${CMAKE_ROOT})

project(SerialPort VERSION 1.0 LANGUAGES CXX)

include(GNUInstallDirs)

# print build options
message(STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
message(STATUS "CMAKE_INSTALL_PREFIX: " ${CMAKE_INSTALL_PREFIX})

# package build
set(SOURCE_FILES
    "src/Serial.cpp"
    "src/Linux_Serial.cpp"
)
add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
                           
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_11)

# error setting
add_library(error_settings INTERFACE)
target_compile_options(
    ${PROJECT_NAME}
    INTERFACE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wfatal-errors>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wfatal-errors>
    $<$<CXX_COMPILER_ID:MSVC>:/W3 /permissive->
)

# optimize setting
add_library(optimize_settings INTERFACE)
target_compile_options(
    ${PROJECT_NAME}
    INTERFACE
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:GNU>>:-O2>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:Clang>>:-O2>
    $<$<AND:$<CONFIG:Release>,$<CXX_COMPILER_ID:MSVC>>:/O2 -DNDEBUG>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:GNU>>:-O0 -g>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:Clang>>:-O0 -g>
    $<$<AND:$<CONFIG:Debug>,$<CXX_COMPILER_ID:MSVC>>:/Zi /FS /DEBUG /Od /MP /MDd /Oy->
)

# create and install find_package
install(TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(FILES include/Serial.hpp DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(
    EXPORT ${PROJECT_NAME}Targets
    FILE ${PROJECT_NAME}Config.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
)

