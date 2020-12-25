cmake_minimum_required(VERSION 3.16)

project(tutorial C CXX)

find_package(Protobuf REQUIRED)
if(Protobuf_FOUND)
    message(STATUS "Using Protobuf ${Protobuf_VERSION}")
else()
    message(FATAL_ERROR "Protobuf not found")
endif()

set(PROTO family.proto)
set(PROTO_DIR ${PROJECT_SOURCE_DIR}/proto)
protobuf_generate_cpp(PROTO_SRC PROTO_HDR ${PROTO_DIR}/${PROTO})
message("PROTO_SRC = ${PROTO_SRC}")
message("PROTO_HDR = ${PROTO_HDR}")

set(CMAKE_CXX_STANDARD 14)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsizeof-pointer-memaccess \
                     -Wfloat-equal -Wconversion-null -Woverflow -Wshadow \
                     -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -pthread -g -O3")

# 关联protobuf到最后的二进制文件
add_executable(${PROJECT_NAME}
        main.cpp
        ${PROTO_SRC}
        ${PROTO_HDR})

include_directories(${PROJECT_NAME}
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
        PUBLIC ${Protobuf_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} ${Protobuf_LIBRARIES})
