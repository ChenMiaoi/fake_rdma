cmake_minimum_required(VERSION 3.20.0)
project(fake-rdma VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

include_directories(
  include
  include/kernel
  include/kernel/uapi

  /usr/include
)

file(GLOB_RECURSE LIB_SOURCES libs/*.c)

add_library(fake-libs ${LIB_SOURCES})
target_link_libraries(fake-libs 
  mnl
)
target_include_directories(fake-libs PUBLIC
  include/libs
)

add_subdirectory(${CMAKE_SOURCE_DIR}/fake_device)
