#include "thread_pool/thread_pool.hpp"

namespace dispatcher::thread_pool {

ThreadPool::ThreadPool(std::shared_ptr<queue::PriorityQueue> prior_q, size_t count_threads) {}

//  wait until end tasks
ThreadPool::~ThreadPool() {}

void ThreadPool::start_threads() {}

}  // namespace dispatcher::thread_pool