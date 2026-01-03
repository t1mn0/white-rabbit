#pragma once

#include <vvv/list.hpp>

#include "../../tasks/task.hpp"

#include <condition_variable>
#include <mutex>
#include <optional>

namespace wr {

// Unbounded
// Blocking
// MP-MC
class GlobalTaskQueue {
  private:  // fields:
    std::mutex mutex_;
    vvv::IntrusiveList<TaskBase> buffer_;
    std::condition_variable not_empty_;

  public:  // member-types:
    using ValueType = TaskBase*;

  public:  // member-functions:
    GlobalTaskQueue() = default;
    ~GlobalTaskQueue() = default;
    GlobalTaskQueue(const GlobalTaskQueue&) = delete;             // non-copyable;
    GlobalTaskQueue& operator=(const GlobalTaskQueue&) = delete;  // non-copyassignable;
    GlobalTaskQueue(GlobalTaskQueue&&) = delete;                  // non-movable;
    GlobalTaskQueue& operator=(GlobalTaskQueue&&) = delete;       // non-moveassignable;

    void push_task(TaskBase* task);
    std::optional<TaskBase*> try_extract_task();

    // docking is process of joining a tasks batch to the global_queue,
    // which is wrapped in an IntrusiveList:
    void dock_list(vvv::IntrusiveList<TaskBase>&& list);

    // undocking is reverse operation to the `dock_list` - serve to pick up
    // tasks batch from the global queue, which wrapped in an IntrusiveList:
    std::optional<vvv::IntrusiveList<TaskBase>> undock(size_t count);
};

}  // namespace wr
