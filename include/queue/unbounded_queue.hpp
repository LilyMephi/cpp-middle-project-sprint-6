#pragma once
#include "queue/queue.hpp"
#include "logger.hpp"

#include <condition_variable>
#include <mutex>
#include <queue>

namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<std::function<void()>> task_queue_;

    mutable std::mutex mutex_;

public:
    explicit UnboundedQueue() = default;

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~UnboundedQueue() override = default;

    bool empty() override;
};

}  // namespace dispatcher::queue