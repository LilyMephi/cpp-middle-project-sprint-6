#include "queue/unbounded_queue.hpp"

#include <functional>
#include <mutex>
#include <queue>
#include <semaphore>

namespace dispatcher::queue {

void UnboundedQueue::push(std::function<void()> task) {
    std::unique_lock<std::mutex> lock(mutex_);
    task_queue_.push(std::move(task));
    lock.unlock();
}

std::optional<std::function<void()>> UnboundedQueue::try_pop() {
    std::unique_lock<std::mutex> lock(mutex_);
    if (task_queue_.empty()) {
        Logger::Get().Log("UnboundedQueue::try_pop - queue empty, returning nullopt");
        return std::nullopt;
    }

    auto task = std::move(task_queue_.front());
    task_queue_.pop();
    lock.unlock();
    return task;
}

bool UnboundedQueue::empty() {
    std::lock_guard<std::mutex> lock(mutex_);
    return task_queue_.empty();
}

}  // namespace dispatcher::queue