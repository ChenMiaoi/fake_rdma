cmake_minimum_required(VERSION 3.20.0)
project(fake-rdma VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(LIBMNL_ROOT ${CMAKE_SOURCE_DIR}/deps/libmnl)

if (NOT EXISTS ${CMAKE_BINARY_DIR}/libmnl/libmnl.so)
  execute_process(
    COMMAND bash prebuild.sh
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR} 
  )

  if(NOT result EQUAL 0)
      message(FATAL_ERROR "Script failed with error: ${error}")
  endif()
else()
  message(STATUS "Found libmnl.so")
endif()

include_directories(
  include
  include/kernel
  include/kernel/uapi

  ${LIBMNL_ROOT}/include

  /usr/include
)

file(GLOB_RECURSE LIB_SOURCES libs/*.c)

add_library(fake-libs ${LIB_SOURCES})
target_link_libraries(fake-libs
  ${CMAKE_BINARY_DIR}/libmnl/libmnl.so
)
target_include_directories(fake-libs PUBLIC
  include/libs
)

add_subdirectory(${CMAKE_SOURCE_DIR}/fake_device)
