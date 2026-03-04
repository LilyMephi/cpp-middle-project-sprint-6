#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(const size_t capacity) : bound_queue(capacity), unbound_queue() {}

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    if (priority == TaskPriority::High) {
        bound_queue.push(task);
    } else {
        unbound_queue.push(task);
    }
    not_empty_.notify_one();
}

std::optional<std::function<void()>> PriorityQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    not_empty_.wait(lock,
                       [this]() { return !bound_queue.empty() || !unbound_queue.empty() || shutdown_flag.load(); });


    if (!bound_queue.empty()) {
        auto item = bound_queue.try_pop();
        if (item)
            return item;
    }
    if (!unbound_queue.empty()) {
        auto item = unbound_queue.try_pop();
        if (item)
            return item;
    }
    return std::nullopt;
}

void PriorityQueue::shutdown() {
    shutdown_flag.store(true);
    not_empty_.notify_all();
}

bool PriorityQueue::empty() {
    return bound_queue.empty() && unbound_queue.empty();
}
}  // namespace dispatcher::queue