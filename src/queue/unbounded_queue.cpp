#include "queue/unbounded_queue.hpp"

#include <functional>
#include <mutex>
#include <queue>
#include <semaphore>

namespace dispatcher::queue {

void UnboundedQueue::push(std::function<void()> task) { task_queue.push(task); }

std::optional<std::function<void()>> UnboundedQueue::try_pop() {
    if (task_queue.empty()) {
        return std::nullopt;
    }

    auto task = std::move(task_queue.front());
    task_queue.pop();
    return task;
}

bool UnboundedQueue::empty() { return task_queue.empty(); }

}  // namespace dispatcher::queue