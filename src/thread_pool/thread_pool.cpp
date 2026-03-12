#include "thread_pool/thread_pool.hpp"

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> prior_q, size_t count_threads)
    : priority_queue_(std::move(prior_q)), num_threads_(count_threads) {
    if (!priority_queue_) {
        throw std::invalid_argument("ThreadPool: priority_queue cannot be null");
    }

    if (count_threads == 0) {
        throw std::invalid_argument("ThreadPool: number of threads must be greater than 0");
    }
    Logger::Get().Log("ThreadPool::ThreadPool - initialized with " + std::to_string(count_threads) + " threads");
    try {
        start_threads();
    } catch (const std::exception &e) {
        Logger::Get().Log("ThreadPool::ThreadPool - failed to start threads: " + std::string(e.what()));
        throw;
    }
}

ThreadPool::~ThreadPool() {
    Logger::Get().Log("ThreadPool::~ThreadPool() - end work ");
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_.store(true);
        priority_queue_->shutdown();
    }

    for (auto &worker : workers_) {
        try {
            worker.join();
        } catch (const std::system_error &e) {
            Logger::Get().Log("ThreadPool::~ThreadPool - join failed: " + std::string(e.what()));
        }
    }

    workers_.clear();
    Logger::Get().Log("ThreadPool::~ThreadPool - shutdown completed successfully");
}

void ThreadPool::start_threads() {
    Logger::Get().Log("ThreadPool::start_threads() - create threads for queueu");
    workers_.reserve(num_threads_);
    for (size_t i = 0; i < num_threads_; ++i) {
        try {
            workers_.emplace_back(&ThreadPool::worker_thread, this);
            Logger::Get().Log("ThreadPool::start_threads - thread " + std::to_string(i) + " created");
        } catch (const std::system_error &e) {
            Logger::Get().Log("ThreadPool::start_threads - failed to create thread " + std::to_string(i) + ": " +
                              std::string(e.what()));
            stop_.store(true);
            priority_queue_->shutdown();
            throw;
        }
    }
}

void ThreadPool::worker_thread() {
    while (true) {
        std::optional<std::function<void()>> task;
        {
            if (stop_.load() && priority_queue_->empty()) {
                Logger::Get().Log("ThreadPool::worker_thread() - queue empty and get flag to stop");
                break;
            }

            task = std::move(priority_queue_->pop());
        }

        if (task.has_value()) {
            try {
                Logger::Get().Log("ThreadPool::worker_thread - executing task");
                task.value()();
                Logger::Get().Log("ThreadPool::worker_thread - task completed");
            } catch (const std::exception &e) {
                Logger::Get().Log("ThreadPool::worker_thread - task failed: " + std::string(e.what()));
            } catch (...) {
                Logger::Get().Log("ThreadPool::worker_thread - task failed with unknown exception");
            }
        } else {
            Logger::Get().Log("ThreadPool::worker_thread - got empty task (likely shutdown)");
        }
    }
    Logger::Get().Log("ThreadPool::worker_thread - worker thread exiting");
}

}  // namespace dispatcher::thread_pool