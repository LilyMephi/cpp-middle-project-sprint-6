#pragma once

#include "queue/priority_queue.hpp"
#include <thread>
namespace dispatcher::thread_pool {

class ThreadPool {
public:
    ThreadPool(std::shared_ptr<queue::PriorityQueue> prior_q, size_t count_threads);

    ~ThreadPool();

private:
    void start_threads();
    void worker_thread();
    void wait();

    std::shared_ptr<queue::PriorityQueue> priority_queue_;
    std::vector<std::thread> workers_;
    std::mutex mutex_;
    std::condition_variable not_empty_;
    std::atomic<bool> stop_{false};
    size_t num_threads_;
};

}  // namespace dispatcher::thread_pool
