#include "task_dispatcher.hpp"

namespace dispatcher {

TaskDispatcher::TaskDispatcher(size_t thread_count, size_t cpacity) : thread_count_(thread_count){
  priority_queue = std::make_shared<queue::PriorityQueue>(cpacity);
  t_pool = std::make_unique<thread_pool::ThreadPool>(priority_queue, thread_count);
}

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task) {
  priority_queue->push(priority, task);
}

}  // namespace dispatcher