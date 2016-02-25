cmake_minimum_required(VERSION 2.8.0)

# ---[ Latte project
project(xHacking C CXX)

# ---[ Includes
set(xHacking_INCLUDE_DIR ${PROJECT_SOURCE_DIR}/include)
include_directories(${xHacking_INCLUDE_DIR} ${PROJECT_BINARY_DIR})

# ---[ Subdirectories
add_subdirectory(src)
