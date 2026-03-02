#pragma once

#include "queue/priority_queue.hpp"
namespace dispatcher::thread_pool {

class ThreadPool {
public:
    ThreadPool(std::shared_ptr<queue::PriorityQueue> prior_q, size_t count_threads);

    ~ThreadPool();
private:
  void start_threads();
};

}  // namespace dispatcher::thread_pool
