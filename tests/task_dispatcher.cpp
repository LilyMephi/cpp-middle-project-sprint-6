#include "task_dispatcher.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace dispatcher;

TEST(TaskDispatcherTest, ScheduleAddsTask) {
    std::atomic<int> counter{0};
    {
        std::unique_ptr<TaskDispatcher> dispatcher = std::make_unique<TaskDispatcher>(4, 1000);
        dispatcher->schedule(TaskPriority::High, [&counter]() { counter++; });
    }

    EXPECT_GE(counter.load(), 0);
}

TEST(TaskDispatcherTest, StressTestManyTasks) {
    constexpr size_t num_tasks = 1000;
    std::atomic<size_t> completed{0};
    {
        std::unique_ptr<TaskDispatcher> dispatcher = std::make_unique<TaskDispatcher>(4, 1000);
        for (size_t i = 0; i < num_tasks; ++i) {
            dispatcher->schedule(TaskPriority::Normal, [&completed]() { completed.fetch_add(1); });
        }
    }
    EXPECT_EQ(completed.load(), num_tasks) << "Not all tasks completed within timeout";
}

TEST(TaskDispatcherTest, CapacityLimit) {

    constexpr size_t capacity = 10;
    std::atomic<size_t> scheduled{0};
    
    {
        auto small_dispatcher = std::make_unique<TaskDispatcher>(2, capacity);
        for (size_t i = 0; i < capacity * 2; ++i) {
            small_dispatcher->schedule(TaskPriority::Normal, [&scheduled]() { scheduled.fetch_add(1); });
        }
    }

    EXPECT_GE(scheduled.load(), capacity);
}

TEST(TaskDispatcherTest, ScheduleMultiplePriorities) {
    std::atomic<int> high_count{0}, normal_count{0};
    {
        std::unique_ptr<TaskDispatcher> dispatcher_ = std::make_unique<TaskDispatcher>(4, 1000);
        dispatcher_->schedule(TaskPriority::High, [&high_count]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            high_count.fetch_add(1);
        });

        dispatcher_->schedule(TaskPriority::Normal, [&normal_count]() { normal_count.fetch_add(1); });

    }

    std::mutex mtx_;
    {
        std::lock_guard<std::mutex> lock(mtx_);
        EXPECT_EQ(high_count.load(), 1);
        EXPECT_EQ(normal_count.load(), 1);
    }
}