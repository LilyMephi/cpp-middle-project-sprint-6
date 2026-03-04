#include "queue/bounded_queue.hpp"

namespace dispatcher::queue {

void BoundedQueue::push(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(mutex_);
    not_full_.wait(lock, [this] { return task_queue_.size() < capacity_; });
    task_queue_.push(std::move(task));
    lock.unlock();
}

std::optional<std::function<void()>> BoundedQueue::try_pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (task_queue_.empty()) {
        Logger::Get().Log("BoundedQueue::try_pop - queue empty, returning nullopt");
        return std::nullopt;
    }

    auto task = std::move(task_queue_.front());
    task_queue_.pop();
    lock.unlock();
    not_full_.notify_one();
    return task;
}

bool BoundedQueue::empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return task_queue_.empty();
}

}  // namespace dispatcher::queue