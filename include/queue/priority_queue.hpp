#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

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

    std::array<std::unique_ptr<IQueue>, 2> queues_;
    mutable std::mutex mutex_;
    std::condition_variable not_shutdown_;
    std::atomic<bool> shutdown_flag{false};

public:
    explicit PriorityQueue(size_t capacity = 1000);

    void push(TaskPriority priority, std::function<void()> task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<std::function<void()>> pop();

    void shutdown();

    size_t get_idx(TaskPriority priority);

    ~PriorityQueue() = default;
};

}  // namespace dispatcher::queue