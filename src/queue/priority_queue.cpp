#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

// explicit PriorityQueue(?);

size_t PriorityQueue::get_idx(TaskPriority priority) { return static_cast<size_t>(priority); }

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    if (shutdown_flag.load())
        return;

    {
        std::unique_lock<std::mutex> lock(mutex_);
        queues[get_idx(priority)]->push(std::move(task));
    }

    cv_.notify_one();
}
// block on pop until shutdown is called
// after that return std::nullopt on empty queue
std::optional<std::function<void()>> PriorityQueue::pop() {
    if (shutdown_flag.load())
        return std::nullopt;
    {
        std::unique_lock<std::mutex> lock(mutex_);

        cv_.wait(lock, [this]() {
            return !queues[get_idx(TaskPriority::Normal)]->empty() || !queues[get_idx(TaskPriority::High)]->empty();
        });
        if (!queues[get_idx(TaskPriority::High)]->empty()) {
            auto item = queues[get_idx(TaskPriority::High)]->try_pop();
            return item;
        } else if (!queues[get_idx(TaskPriority::Normal)]->empty()) {
            auto item = queues[get_idx(TaskPriority::Normal)]->try_pop();
            return item;
        } else {
            return std::nullopt;
        }
    }
}

void PriorityQueue::shutdown() {
    shutdown_flag.store(true);
    cv_.notify_all();
}

}  // namespace dispatcher::queue