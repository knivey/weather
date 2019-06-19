cmake_minimum_required(VERSION 3.7)
project(weather)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
#ubuntu puts files in a stupid place
set(cpprestsdk_DIR /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/cmake/)
find_package(cpprestsdk REQUIRED NAMES cpprestsdk cpprest)

add_executable(weather main.cpp weather.cpp location.cpp FetchURL.cpp GetTime.cpp grpc/ultimateq.grpc.pb.cc grpc/ultimateq.pb.cc)

target_link_libraries(weather PRIVATE cpprestsdk::cpprest fmt grpc++ protobuf)
install(TARGETS weather RUNTIME DESTINATION bin)

file(COPY ${CMAKE_SOURCE_DIR}/keys/knivey.crt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/keys/knivey.key DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/keys/extra.crt DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/keys/darksky_key DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/keys/bing_key DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

add_subdirectory(3rdparty/googletest)

include_directories(${gtest_SOURCE_DIR}/include/
                    3rdparty
                    )
enable_testing()
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} -g -Wall")
add_executable(tests weather.cpp location.cpp GetTime.cpp test/weather_test.cpp test/FileReaderTest.cpp test/GetTimeTest.cpp)
target_link_libraries(tests gtest cpprestsdk::cpprest fmt)
add_test(NAME weather COMMAND weather_test)
