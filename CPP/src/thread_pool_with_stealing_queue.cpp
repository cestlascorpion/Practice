#include "thread_pool_with_stealing_queue.h"

template <typename task_type>
thread_local work_stealing_queue<task_type> *thread_pool_with_stealing_queue<task_type>::local_work_queue = nullptr;

template <typename task_type>
thread_local unsigned thread_pool_with_stealing_queue<task_type>::my_index = 0;
