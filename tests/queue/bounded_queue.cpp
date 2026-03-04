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

TEST_F(BoundedQueueTest, MultiplePushPop) {
    std::vector<int> expected = {1, 2, 3};

    for (int val : expected) {
        queue_->push(create_task(val));
    }

    for (int i = 0; i < expected.size(); ++i) {
        auto task = queue_->try_pop();
        ASSERT_TRUE(task.has_value());
    }

    EXPECT_TRUE(queue_->empty());
}

TEST_F(BoundedQueueTest, PushBlocksWhenFull) {
    for (int i = 0; i < capacity_; ++i) {
        queue_->push(create_task(i));
    }

    auto push_start_task = std::thread([&]() { queue_->push(create_task(99)); });

    auto task = queue_->try_pop();
    ASSERT_TRUE(task.has_value());

    push_start_task.join();
}
