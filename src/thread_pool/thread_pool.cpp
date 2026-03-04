#include "thread_pool/thread_pool.hpp"

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> prior_q, size_t count_threads)
    : priority_queue_(std::move(prior_q)), num_threads_(count_threads) {
    start_threads();
}

//  wait until end tasks
ThreadPool::~ThreadPool() {
    {
        std::lock_guard<std::mutex> lock(mutex_);
        // if (stop_.load()) return;
        stop_.store(true);
        priority_queue_->shutdown();
        not_empty_.notify_all();
    }
    for (auto &worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }

    workers_.clear();
}

void ThreadPool::start_threads() {
    workers_.reserve(num_threads_);
    for (size_t i = 0; i < num_threads_; ++i) {
        workers_.emplace_back(&ThreadPool::worker_thread, this);
    }
}

void ThreadPool::worker_thread() {
    while (true) {
        std::optional<std::function<void()>> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            not_empty_.wait(lock, [this] { return !priority_queue_->empty() || stop_.load(); });

            if (stop_.load() && priority_queue_->empty()) {
                break;
            }

            task = std::move(priority_queue_->pop());
        }

        if (task.has_value()) {
            task.value()();
        }
    }
}

}  // namespace dispatcher::thread_pool