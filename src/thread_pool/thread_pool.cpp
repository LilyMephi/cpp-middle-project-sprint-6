#include "thread_pool/thread_pool.hpp"

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> prior_q, size_t count_threads)
    : priority_queue_(std::move(prior_q)), num_threads_(count_threads) {
    start_threads();
}

//  wait until end tasks
ThreadPool::~ThreadPool() {
    stop_.store(true);
    not_empty_.notify_all();
    priority_queue_->shutdown();

    for (auto &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::start_threads() {
    workers_.reserve(num_threads_);
    for (size_t i = 0; i < num_threads_; ++i) {
        workers_.emplace_back(&ThreadPool::worker_thread, this);
    }
}

void ThreadPool::worker_thread() {
    while (!stop_) {
        std::optional<std::function<void()>> task = priority_queue_->pop();
        if (task.has_value()) {
            task.value()();
        } else {
            std::this_thread::yield();
        }
    }
}

}  // namespace dispatcher::thread_pool