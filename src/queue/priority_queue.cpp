#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(const size_t capacity) : bound_queue(capacity), unbound_queue() {
    if (capacity == 0) {
        Logger::Get().Log("PriorityQueue capacity must be greater than 0");
        throw;
    }

    Logger::Get().Log("PriorityQueue::PriorityQueue - initialized with capacity: " + std::to_string(capacity));
}

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    try {
        if (priority == TaskPriority::High) {
            bound_queue.push(task);
        } else {
            unbound_queue.push(task);
        }
    } catch (const std::exception &e) {
        Logger::Get().Log("PriorityQueue::push - exception: " + std::string(e.what()));
        return;
    }
    not_empty_.notify_one();
}

std::optional<std::function<void()>> PriorityQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    not_empty_.wait(lock, [this]() { return !bound_queue.empty() || !unbound_queue.empty() || shutdown_flag.load(); });

    auto item_bound = bound_queue.try_pop();
    if (item_bound)
        return item_bound;

    auto item_unbound = unbound_queue.try_pop();
    if (item_unbound)
        return item_unbound;

    Logger::Get().Log("PriorityQueue::pop - queue empty, returning nullopt");
    return std::nullopt;
}

void PriorityQueue::shutdown() {
    Logger::Get().Log("PriorityQueue::shutdown - shutdown priority queue");
    std::lock_guard<std::mutex> lock(mutex_);
    shutdown_flag.store(true);
    not_empty_.notify_all();
}

bool PriorityQueue::empty() { return bound_queue.empty() && unbound_queue.empty(); }

}  // namespace dispatcher::queue