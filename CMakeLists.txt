cmake_minimum_required(VERSION 3.13)

project(devices-demo C CXX)

include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup()

add_compile_definitions(DEMO_PORT="1911")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

add_executable(device device.cxx message.cxx)
add_executable(server server.cxx message.cxx)
add_executable(test message.cxx message.unittest.cxx)

target_link_libraries(device ${CONAN_LIBS})
target_link_libraries(server ${CONAN_LIBS})
target_link_libraries(test ${CONAN_LIBS})

set_property(TARGET device PROPERTY CXX_STANDARD 17)
set_property(TARGET server PROPERTY CXX_STANDARD 17)
set_property(TARGET test PROPERTY CXX_STANDARD 17)
