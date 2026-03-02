#include "task_dispatcher.hpp"

namespace dispatcher {

TaskDispatcher::TaskDispatcher(size_t thread_count){}

void TaskDispatcher::schedule(TaskPriority priority, std::function<void()> task){}
TaskDispatcher::~TaskDispatcher() {}

}  // namespace dispatcher