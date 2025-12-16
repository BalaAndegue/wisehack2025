cmake_minimum_required(VERSION 3.10)
project(wisehack25_engine C CXX)

# Indique explicitement où se trouve DynamoRIO
set(DynamoRIO_DIR "/home/bala/DynamoRIO-Linux-11.3.0-1/cmake")
find_package(DynamoRIO REQUIRED)

add_library(engine SHARED src/engine.cpp)

# Configuration DynamoRIO
configure_DynamoRIO_client(engine)

# Extensions nécessaires
use_DynamoRIO_extension(engine drmgr)
use_DynamoRIO_extension(engine drutil)
