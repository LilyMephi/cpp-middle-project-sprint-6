#include "queue/bounded_queue.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace dispatcher::queue;

class BoundedQueueTest : public ::testing::Test {
protected:
    void SetUp() override { queue_ = std::make_unique<BoundedQueue>(capacity_); }

    std::unique_ptr<BoundedQueue> queue_;
    static constexpr int capacity_ = 3;

    std::function<void()> create_task(int value) {
        return [value]() { /* Mock task */ };
    }
};

TEST_F(BoundedQueueTest, InitiallyEmpty) { EXPECT_TRUE(queue_->empty()); }

TEST_F(BoundedQueueTest, PushSingleTask) {
    auto task = create_task(1);
    queue_->push(std::move(task));
    EXPECT_FALSE(queue_->empty());
}

TEST_F(BoundedQueueTest, TryPopEmptyReturnsNullopt) {
    auto result = queue_->try_pop();
    EXPECT_FALSE(result.has_value());
}

TEST_F(BoundedQueueTest, PushAndPopWorks) {
    auto task = create_task(1);
    queue_->push(std::move(task));

    auto result = queue_->try_pop();
    EXPECT_TRUE(result.has_value());
    EXPECT_TRUE(queue_->empty());
}

// TEST_F(BoundedQueueTest, CapacityLimit) {
//     // Fill queue to capacity
//     for (int i = 0; i < capacity_; ++i) {
//         queue_->push(create_task(i));
//     }

//     EXPECT_FALSE(queue_->empty());

//     // Try to push one more - should block but complete within timeout for test
//     auto overflow_task = create_task(99);
//     queue_->push(std::move(overflow_task));  // This will block until space available

//     EXPECT_EQ(queue_->empty(), false);
// }

TEST_F(BoundedQueueTest, MultiplePushPop) {
    std::vector<int> expected = {1, 2, 3};

    // Push tasks
    for (int val : expected) {
        queue_->push(create_task(val));
    }

    // Pop and verify
    for (int i = 0; i < expected.size(); ++i) {
        auto task = queue_->try_pop();
        ASSERT_TRUE(task.has_value());
    }

    EXPECT_TRUE(queue_->empty());
}

// Concurrency test
TEST_F(BoundedQueueTest, ConcurrentPushPop) {
    std::atomic<int> completed_tasks{0};
    constexpr int num_tasks = 10;
    constexpr int num_threads = 4;

    std::vector<std::thread> producers, consumers;

    // Producer threads
    for (int i = 0; i < num_threads; ++i) {
        producers.emplace_back([&, tid = i]() {
            for (int j = 0; j < num_tasks / num_threads; ++j) {
                queue_->push(create_task(tid * 10 + j));
            }
        });
    }

    // Consumer threads
    for (int i = 0; i < num_threads; ++i) {
        consumers.emplace_back([&, tid = i]() {
            for (int j = 0; j < num_tasks / num_threads; ++j) {
                auto task = queue_->try_pop();
                if (task) {
                    ++completed_tasks;
                }
            }
        });
    }

    // Wait for completion with timeout
    auto start = std::chrono::steady_clock::now();
    for (auto &t : producers)
        t.join();
    for (auto &t : consumers)
        t.join();

    EXPECT_EQ(completed_tasks, num_tasks);
}

// Test blocking behavior (needs timeout handling)
TEST_F(BoundedQueueTest, PushBlocksWhenFull) {
    // Fill queue completely
    for (int i = 0; i < capacity_; ++i) {
        queue_->push(create_task(i));
    }

    auto start = std::chrono::steady_clock::now();
    auto push_start_task = std::thread([&]() {
        queue_->push(create_task(99));  // This should block
    });

    // Pop one to make space
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    auto task = queue_->try_pop();
    ASSERT_TRUE(task.has_value());

    // Now push should complete
    push_start_task.join();

    auto duration = std::chrono::steady_clock::now() - start;
    EXPECT_GT(duration, std::chrono::milliseconds(30));  // Blocked for some time
}

TEST_F(BoundedQueueTest, TryPopNonBlocking) {
    auto start = std::chrono::steady_clock::now();
    auto result = queue_->try_pop();
    auto duration = std::chrono::steady_clock::now() - start;

    EXPECT_FALSE(result.has_value());
    EXPECT_LT(duration, std::chrono::milliseconds(10));  // Non-blocking
}
