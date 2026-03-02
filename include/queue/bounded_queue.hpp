#pragma once
#include "queue/queue.hpp"
#include <queue>
namespace dispatcher::queue {

class BoundedQueue : public IQueue {
    std::queue<std::function<void()>> task_queue;
    int capacity;

public:
    explicit BoundedQueue(int capacity);

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~BoundedQueue() override = default;

    bool empty();
};

}  // namespace dispatcher::queue