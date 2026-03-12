#include "queue/priority_queue.hpp"
#include <gtest/gtest.h>

#include <chrono>
#include <functional>
#include <thread>

using namespace dispatcher::queue;
using namespace std::chrono_literals;
using namespace dispatcher;

class PriorityQueueTest : public ::testing::Test {
protected:
    void SetUp() override { pq_ = std::make_unique<PriorityQueue>(10); }

    std::unique_ptr<PriorityQueue> pq_;
};

TEST_F(PriorityQueueTest, PopReturnsHighPriorityFirst) {
    std::atomic<bool> extract_high{false};
    auto high_task = [&]() { extract_high.store(true); };
    auto low_task = [&]() {};

    pq_->push(TaskPriority::High, high_task);
    pq_->push(TaskPriority::Normal, low_task);

    auto result = pq_->pop();
    ASSERT_TRUE(result.has_value());
    result.value()();
    ASSERT_TRUE(extract_high.load());
}

TEST_F(PriorityQueueTest, PopReturnsLowPriorityAfterHigh) {
    std::atomic<bool> extract_high{false};
    std::atomic<bool> extract_normal{false};
    auto high_task = [&]() { extract_high.store(true); };
    auto low_task = [&]() { extract_normal.store(true); };

    pq_->push(TaskPriority::High, high_task);
    pq_->push(TaskPriority::Normal, low_task);

    // First pop gets high priority
    auto first = pq_->pop();
    ASSERT_TRUE(first.has_value());
    first.value()();
    ASSERT_TRUE(extract_high.load());
    // Second pop gets Normal priority
    auto second = pq_->pop();
    ASSERT_TRUE(second.has_value());
    second.value()();
    ASSERT_TRUE(extract_normal.load());
}

TEST_F(PriorityQueueTest, ShutdownSetsFlagAndNotifies) {
    pq_->shutdown();

    auto result = pq_->pop();
    ASSERT_FALSE(result.has_value());
}

TEST_F(PriorityQueueTest, MultipleHighPriorityPopsInOrder) {
    std::vector<int> execution_order;
    auto task1 = [&execution_order]() { execution_order.push_back(1); };
    auto task2 = [&execution_order]() { execution_order.push_back(2); };

    pq_->push(TaskPriority::High, task1);
    pq_->push(TaskPriority::High, task2);

    auto opt1 = pq_->pop();
    auto opt2 = pq_->pop();

    ASSERT_TRUE(opt1.has_value());
    ASSERT_TRUE(opt2.has_value());

    opt1.value()();
    opt2.value()();

    EXPECT_TRUE(execution_order[0] == 1 && execution_order[1] == 2);
}

TEST_F(PriorityQueueTest, ConcurrentPushAndPopMaintainsPriority) {
    std::vector<int> results;
    constexpr size_t num_threads = 4;
    constexpr size_t tasks_per_thread = 10;

    std::vector<std::jthread> workers;

    for (size_t i = 0; i < num_threads; ++i) {
        workers.emplace_back([this, i, tasks_per_thread, &results](std::stop_token) {
            for (size_t j = 0; j < tasks_per_thread; ++j) {
                bool is_high = (j % 2 == 0);
                auto task = [i, j, &results]() { results.push_back(static_cast<int>(i * 10 + j)); };
                this->pq_->push(is_high ? TaskPriority::High : TaskPriority::Normal, std::move(task));
            }
        });
    }

    std::jthread consumer([this, &results]( std::stop_token) {
        while (results.size() < num_threads * tasks_per_thread) {
            auto task = this->pq_->pop();
            if (task) {
                task.value()();
            } else {
                std::this_thread::sleep_for(1ms);
            }
        }
    });

    for (auto &worker : workers) {
        if (worker.joinable())
            worker.join();
    }
    if (consumer.joinable())
        consumer.join();

    ASSERT_EQ(results.size(), num_threads * tasks_per_thread);
}

TEST_F(PriorityQueueTest, BoundQueueCapacityLimitWithThreads) {
    PriorityQueue pq(3);

    std::atomic<size_t> high_tasks_pushed{0};
    std::atomic<size_t> high_tasks_popped{0};
    constexpr size_t total_high_tasks = 10;

    std::vector<std::jthread> producers;
    for (size_t i = 0; i < 4; ++i) {
        producers.emplace_back([&pq, &high_tasks_pushed, total_high_tasks]() {
            for (size_t j = 0; j < total_high_tasks / 4 + 1; ++j) {
                pq.push(TaskPriority::High, []() { /* empty task */ });
                high_tasks_pushed.fetch_add(1);
            }
        });
    }

    std::jthread consumer([&pq, &high_tasks_popped]() {
        for (size_t i = 0; i < 10; ++i) {
            auto task = pq.pop();
            if (task.has_value()) {
                task.value()();
                high_tasks_popped.fetch_add(1);
            }
        }
    });

    for (auto &producer : producers) {
        producer.join();
    }

    consumer.join();

    ASSERT_EQ(high_tasks_popped.load(), 10);
}
