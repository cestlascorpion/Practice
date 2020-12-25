cmake_minimum_required(VERSION 3.16)

project(spider)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Wsizeof-pointer-memaccess \
                     -Wfloat-equal -Wconversion-null -Woverflow -Wshadow \
                     -D_FILE_OFFSET_BITS=64 -D_GNU_SOURCE -pthread -g -O0")

include_directories(src /usr/local/include)

link_directories(/usr/local/lib)

add_executable(spider
        test/spider.cpp
        src/any_type.h
        src/ioc_container.h)

target_link_libraries(spider boost_thread-mt boost_system)

add_executable(sorter
        test/sorter.cpp
        src/function_wrapper.h
        src/join_threads.h
        src/thread_safe_queue.h
        src/thread_pool_with_stealing_queue.h
        src/thread_safe_stack.h
        src/parallel_quick_sort.h)

target_link_libraries(sorter boost_thread boost_system)

add_executable(tester test/tester.cpp src/leaky_bucket.h src/token_bucket.h)

target_link_libraries(tester boost_thread boost_system)