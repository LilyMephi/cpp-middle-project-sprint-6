#pragma once
#include "queue/bounded_queue.hpp"
#include "queue/unbounded_queue.hpp"
#include "types.hpp"

#include <atomic>
#include <limits>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_map>
#include <condition_variable>

namespace dispatcher::queue {

class PriorityQueue {
    static inline const std::array<QueueOptions, 2> priority_configs = {{{false, std::nullopt}, {false, 100}}};

    std::array<std::unique_ptr<IQueue>, 2> queues;
    std::condition_variable cv_;
    mutable std::mutex mutex_;
    std::atomic<bool> shutdown_flag{false};

public:
    // explicit PriorityQueue(?);

    void push(TaskPriority priority, std::function<void()> task);
    // block on pop until shutdown is called
    // after that return std::nullopt on empty queue
    std::optional<std::function<void()>> pop();

    void shutdown();

    size_t get_idx(TaskPriority priority);

    ~PriorityQueue() = default;
};

}  // namespace dispatcher::queue