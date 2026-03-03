#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

#include "queue/priority_queue.hpp"
#include "thread_pool/thread_pool.hpp"
#include "types.hpp"

namespace dispatcher {

class TaskDispatcher {
    std::shared_ptr<queue::PriorityQueue> priority_queue;
    std::unique_ptr<thread_pool::ThreadPool> t_pool;
    size_t thread_count_;

public:
    // TaskDispatcher(size_t thread_count);
    TaskDispatcher(size_t thread_count, size_t cpacity = 1000);
    void schedule(TaskPriority priority, std::function<void()> task);
    ~TaskDispatcher() = default;
};

}  // namespace dispatcher