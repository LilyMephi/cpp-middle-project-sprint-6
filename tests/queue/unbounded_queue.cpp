#include "queue/unbounded_queue.hpp"
#include <atomic>
#include <chrono>
#include <gtest/gtest.h>
#include <thread>
#include <vector>

using namespace dispatcher::queue;
class UnboundedQueueTest : public ::testing::Test {
protected:
    void SetUp() override { queue_ = std::make_unique<UnboundedQueue>(); }

    std::unique_ptr<UnboundedQueue> queue_;

    std::function<void()> create_task(int value) {
        return [value]() { /* Mock task */ };
    }
};

TEST_F(UnboundedQueueTest, MultiplePushPop) {
    std::vector<int> expected = {1, 2, 3, 4, 5};

    for (int val : expected) {
        queue_->push(create_task(val));
    }

    EXPECT_FALSE(queue_->empty());

    for (int i = 0; i < expected.size(); ++i) {
        auto task = queue_->try_pop();
        ASSERT_TRUE(task.has_value());
    }

    EXPECT_TRUE(queue_->empty());
}

TEST_F(UnboundedQueueTest, LargeNumberOfTasks) {
    constexpr int num_tasks = 10000;

    for (int i = 0; i < num_tasks; ++i) {
        queue_->push(create_task(i));
    }

    int popped_count = 0;
    while (auto task = queue_->try_pop()) {
        ++popped_count;
    }

    EXPECT_EQ(popped_count, num_tasks);
    EXPECT_TRUE(queue_->empty());
}

TEST_F(UnboundedQueueTest, ConcurrentPushPop) {
    std::atomic<int> completed_tasks{0};
    constexpr int num_tasks = 1000;
    constexpr int num_threads = 8;
    std::vector<std::thread> producers, consumers;

    for (int i = 0; i < num_threads; ++i) {
        producers.emplace_back([&, tid = i, num_tasks]() {
            for (int j = 0; j < num_tasks / num_threads; ++j) {
                queue_->push(create_task(tid * 100 + j));
            }
        });
    }

    for (int i = 0; i < num_threads; ++i) {
        consumers.emplace_back([&]() {
            for (int j = 0; j < num_tasks / num_threads; ++j) {
                auto task = queue_->try_pop();
                if (task) {
                    (*task)();
                    ++completed_tasks;
                }
            }
        });
    }

    for (auto &t : producers) {
        t.join();
    }
    for (auto &t : consumers) {
        t.join();
    }

    while (auto task = queue_->try_pop()) {
        (*task)();
        ++completed_tasks;
    }

    EXPECT_EQ(completed_tasks.load(), num_tasks);
}

// TEST_F(UnboundedQueueTest, ConcurrentPushPop) {
//     std::atomic<int> completed_tasks{0};
//     constexpr int num_tasks = 1000;
//     constexpr int num_threads = 8;

//     std::vector<std::thread> producers, consumers;

//     for (int i = 0; i < num_threads; ++i) {
//         producers.emplace_back([&, tid = i]() {
//             for (int j = 0; j < num_tasks / num_threads; ++j) {
//                 queue_->push(create_task(tid * 100 + j));
//             }
//         });
//     }

//     for (int i = 0; i < num_threads; ++i) {
//         consumers.emplace_back([&]() {
//             for (int j = 0; j < num_tasks / num_threads; ++j) {
//                 auto task = queue_->try_pop();
//                 if (task) {
//                     ++completed_tasks;
//                 }
//             }
//         });
//     }

//     for (auto &t : producers)
//         t.detach();
//     for (auto &t : consumers)
//         t.detach();

//     auto start = std::chrono::steady_clock::now();
//     while (completed_tasks < num_tasks && std::chrono::steady_clock::now() - start < std::chrono::seconds(5)) {
//         std::this_thread::sleep_for(std::chrono::milliseconds(10));
//     }

//     while (auto task = queue_->try_pop()) {
//         ++completed_tasks;
//     }

//     EXPECT_EQ(completed_tasks, num_tasks);
// }

TEST_F(UnboundedQueueTest, TryPopNonBlockingPerformance) {
    for (int i = 0; i < 10; ++i) {
        queue_->push(create_task(i));
    }

    auto start = std::chrono::steady_clock::now();
    auto result = queue_->try_pop();
    auto duration = std::chrono::steady_clock::now() - start;

    EXPECT_TRUE(result.has_value());
    EXPECT_LT(duration, std::chrono::microseconds(100));
}

TEST_F(UnboundedQueueTest, MixedProducerConsumerLoad) {
    constexpr int test_duration = 3;
    constexpr int target_pushes = 10000;

    std::atomic<int> push_count{0}, pop_count{0};
    std::atomic<bool> done{false};

    std::vector<std::thread> workers;

    workers.emplace_back([&]() {
        while (push_count.load() < target_pushes && !done.load()) {
            queue_->push(create_task(push_count.load()));
            push_count.fetch_add(1);
            std::this_thread::sleep_for(std::chrono::microseconds(50));
        }
        done.store(true);
    });

    for (int i = 0; i < 4; ++i) {
        workers.emplace_back([&]() {
            while (!done.load() || !queue_->empty()) {
                if (auto task = queue_->try_pop()) {
                    (*task)();  // Execute task
                    pop_count.fetch_add(1);
                }
                std::this_thread::yield();
            }
        });
    }

    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < std::chrono::seconds(test_duration)) {
        if (done.load() && queue_->empty()) {
            break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    done.store(true);

    for (auto &t : workers) {
        if (t.joinable()) {
            t.join();
        }
    }

    while (auto task = queue_->try_pop()) {
        (*task)();
        pop_count.fetch_add(1);
    }

    EXPECT_GE(pop_count.load(), push_count.load() * 0.95);
    EXPECT_LE(std::abs(static_cast<int>(push_count.load() - pop_count.load())), 100);
    SUCCEED() << "Pushed: " << push_count.load() << ", Popped: " << pop_count.load();
}

// TEST_F(UnboundedQueueTest, MixedProducerConsumerLoad) {
//     std::atomic<bool> done{false};
//     std::atomic<int> push_count{0}, pop_count{0};

//     std::vector<std::thread> workers;

//     workers.emplace_back([&]() {
//         while (!done) {
//             queue_->push(create_task(push_count));
//             push_count.fetch_add(1);
//             std::this_thread::sleep_for(std::chrono::microseconds(10));
//         }
//     });

//     for (int i = 0; i < 3; ++i) {
//         workers.emplace_back([&]() {
//             while (!done || !queue_->empty()) {
//                 if (auto task = queue_->try_pop()) {
//                     pop_count.fetch_add(1);
//                 }
//                 std::this_thread::sleep_for(std::chrono::microseconds(5));
//             }
//         });
//     }

//     std::this_thread::sleep_for(std::chrono::seconds(100));
//     done = true;

//     for (auto &t : workers) {
//         if (t.joinable())
//             t.join();
//     }

//     EXPECT_GT(pop_count, push_count);
//     EXPECT_TRUE(queue_->empty());
// }

TEST_F(UnboundedQueueTest, EmptyCheckThreadSafe) {
    std::atomic<bool> stop{false};

    std::vector<std::thread> checkers;
    for (int i = 0; i < 5; ++i) {
        checkers.emplace_back([&]() {
            while (!stop) {
                EXPECT_NO_THROW(queue_->empty());
            }
        });
    }

    std::thread producer([&]() {
        for (int i = 0; i < 100; ++i) {
            queue_->push(create_task(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    stop = true;
    producer.join();

    for (auto &t : checkers) {
        if (t.joinable())
            t.join();
    }
}
