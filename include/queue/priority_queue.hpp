#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"
#include "logger.hpp"

#include <atomic>
#include <condition_variable>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>

namespace dispatcher::queue {

class PriorityQueue {

    BoundedQueue bound_queue;
    UnboundedQueue unbound_queue;

    mutable std::mutex mutex_;
    std::condition_variable not_empty_;
    std::atomic<bool> shutdown_flag{false};

public:
    PriorityQueue(const PriorityQueue &) = delete;
    PriorityQueue &operator=(const PriorityQueue &) = delete;

    explicit PriorityQueue(size_t capacity = 1000);

    void push(TaskPriority priority, std::function<void()> task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<std::function<void()>> pop();

    void shutdown();
    bool empty();
     ~PriorityQueue() = default;
};

}  // namespace dispatcher::queue