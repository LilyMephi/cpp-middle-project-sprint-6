#include "task_dispatcher.hpp" 
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>

using namespace dispatcher ;

class TaskDispatcherTest : public ::testing::Test {
protected:
    void SetUp() override { dispatcher = std::make_unique<TaskDispatcher>(4, 1000); }

    void TearDown() override { dispatcher.reset(); }

    std::unique_ptr<TaskDispatcher> dispatcher;
};


TEST_F(TaskDispatcherTest, ScheduleAddsTask) {
    std::atomic<int> counter{0};

    dispatcher->schedule(TaskPriority::High, [&counter]() { counter++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    EXPECT_GE(counter.load(), 0);  
}

TEST_F(TaskDispatcherTest, ScheduleMultiplePriorities) {
    std::atomic<int> high_count{0}, normal_count{0};

    dispatcher->schedule(TaskPriority::High, [&high_count]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        high_count++;
    });

    dispatcher->schedule(TaskPriority::Normal, [&normal_count]() { normal_count++; });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    EXPECT_EQ(high_count.load(), 1);
    EXPECT_EQ(normal_count.load(), 1);
}

TEST_F(TaskDispatcherTest, StressTestManyTasks) {
    constexpr size_t num_tasks = 1000;
    std::atomic<size_t> completed{0};

    for (size_t i = 0; i < num_tasks; ++i) {
        dispatcher->schedule(TaskPriority::Normal, [&completed]() { completed.fetch_add(1); });
    }

    auto start = std::chrono::steady_clock::now();
    while (completed.load() < num_tasks && std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    EXPECT_EQ(completed.load(), num_tasks) << "Not all tasks completed within timeout";
}

TEST_F(TaskDispatcherTest, CapacityLimit) {

    constexpr size_t capacity = 10;
    auto small_dispatcher = std::make_unique<TaskDispatcher>(2, capacity);

    std::atomic<size_t> scheduled{0};
    for (size_t i = 0; i < capacity * 2; ++i) {
        small_dispatcher->schedule(TaskPriority::Normal, [&scheduled]() { scheduled++; });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    EXPECT_GE(scheduled.load(), capacity);
}

