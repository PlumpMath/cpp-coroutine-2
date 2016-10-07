cmake_minimum_required(VERSION 3.6)
project(asio_redis)
link_directories(/usr/local/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(LIBRARIES libboost_coroutine.a libboost_system.a)
set(SOURCE_FILES ./test/test.cpp ./src/asioRedis.cpp ./include/asioRedis.h)
link_libraries("/usr/local/lib/libboost_system.a"
               "/usr/local/lib/libboost_coroutine.a"
               "/usr/local/lib/libboost_context.a"
               "/usr/local/lib/libboost_unit_test_framework.a")
add_executable(asio_redis ${SOURCE_FILES})
