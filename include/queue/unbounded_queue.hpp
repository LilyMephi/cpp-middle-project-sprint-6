#pragma once
#include "queue/queue.hpp"
#include <queue>
namespace dispatcher::queue {

class UnboundedQueue : public IQueue {
    std::queue<std::function<void()>> task_queue;
public:
    explicit UnboundedQueue(int capacity) {};

    void push(std::function<void()> task) override;

    std::optional<std::function<void()>> try_pop() override;

    ~UnboundedQueue() override = default;

    bool empty();
};

}  // namespace dispatcher::queue