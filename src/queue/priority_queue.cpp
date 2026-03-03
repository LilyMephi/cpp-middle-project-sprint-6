#include "queue/priority_queue.hpp"

namespace dispatcher::queue {

PriorityQueue::PriorityQueue(const size_t capacity) {
  queues_[get_idx(TaskPriority::High)] =  std::make_unique<BoundedQueue>(capacity);
  queues_[get_idx(TaskPriority::Normal)] =  std::make_unique<UnboundedQueue>();
}

size_t PriorityQueue::get_idx(TaskPriority priority) { return static_cast<size_t>(priority); }

void PriorityQueue::push(TaskPriority priority, std::function<void()> task) {
    queues_[get_idx(priority)]->push(std::move(task));
}

std::optional<std::function<void()>> PriorityQueue::pop() {
    std::unique_lock<std::mutex> lock(mutex_);

    not_shutdown_.wait(lock, [this]() {
        return shutdown_flag.load() || !queues_[get_idx(TaskPriority::Normal)]->empty() ||
               !queues_[get_idx(TaskPriority::High)]->empty();
    });
    
    if (!queues_[get_idx(TaskPriority::High)]->empty()) {
        auto item = queues_[get_idx(TaskPriority::High)]->try_pop();
        return item;
    } else if (!queues_[get_idx(TaskPriority::Normal)]->empty()) {
        auto item = queues_[get_idx(TaskPriority::Normal)]->try_pop();
        return item;
    } else {
        return std::nullopt;
    }
}

void PriorityQueue::shutdown() {
    shutdown_flag.store(true);
    not_shutdown_.notify_all();
}

}  // namespace dispatcher::queue