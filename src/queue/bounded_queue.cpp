#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {

BoundedQueue::BoundedQueue(int capacity) : capacity(capacity) {}

void BoundedQueue::push(std::function<void()> task) {
    if (task_queue.size() >= capacity)
        return;
    task_queue.push(task);
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    if (task_queue.empty()) {
        return std::nullopt;
    }

    auto task = std::move(task_queue.front());
    task_queue.pop();
    return task;
}

bool BoundedQueue::empty() { return task_queue.empty(); }
// BoundedQueue::~BoundedQueue() {}

}  // namespace dispatcher::queue