cmake_minimum_required(VERSION 3.20.0)
project(fake-device VERSION 0.1.0 LANGUAGES C)

set(CMAKE_C_FLAGS 
  "-Wall -Wstrict-prototypes  -Wmissing-prototypes -Wmissing-declarations -Wold-style-definition -Wformat=2 -O2 -pipe -g")
set(LD_FLAGS mnl)

add_compile_definitions(GNU_SOURCE)
add_compile_definitions(FILE_OFFSET_BITS=64)
add_compile_definitions(LARGEFILE_SOURCE)
add_compile_definitions(LARGEFILE64_SOURCE)

include_directories(include)
include_directories(kernel/uapi)

add_executable(fake-rdma
  src/rdma.c
  src/rdma_utils.c
)

target_link_libraries(fake-rdma 
  fake-libs
  ${LD_FLAGS}
)
