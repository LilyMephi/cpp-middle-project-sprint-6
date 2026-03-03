#pragma once
#include "queue/queue.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<std::function<void()>> task_queue_;

    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;

public:
    explicit UnboundedQueue() {};

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~UnboundedQueue() override = default;

    bool empty() override;
};

}  // namespace dispatcher::queue