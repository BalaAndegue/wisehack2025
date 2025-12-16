cmake_minimum_required(VERSION 3.10)
project(wisehack25_engine)

set(DynamoRIO_DIR ~/DynamoRIO-Linux-11.3.0-1/cmake)
find_package(DynamoRIO REQUIRED)

add_library(engine SHARED src/engine.cpp)
configure_DynamoRIO_client(engine)
