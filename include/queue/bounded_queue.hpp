#pragma once
#include "queue/queue.hpp"
#include <queue>
#include <mutex>
#include <condition_variable>
namespace dispatcher::queue {

class BoundedQueue : public IQueue {
    std::queue<std::function<void()>> task_queue_;
    int capacity_;

    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;

public:
    explicit BoundedQueue(int capacity) : capacity_(capacity) {}

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~BoundedQueue() override = default;

    bool empty()  override;
};

}  // namespace dispatcher::queue