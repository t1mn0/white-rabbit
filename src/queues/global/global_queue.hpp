#pragma once

#include <vvv/list.hpp>

#include "../../tasks/concept.hpp"

#include <condition_variable>
#include <mutex>
#include <optional>

namespace wr {

// 1. Unbounded
// 2. Blocking
// 3. MP-MC
template <task::Task TaskT>
class GlobalTaskQueue {
  private:  // fields:
    std::mutex mutex_;
    vvv::IntrusiveList<TaskT> buffer_;
    std::condition_variable not_empty_;

  public:  // member-types:
    using ValueType = TaskT*;

  public:  // member-functions:
    GlobalTaskQueue() = default;
    ~GlobalTaskQueue() = default;
    GlobalTaskQueue(const GlobalTaskQueue&) = delete;             // non-copyable;
    GlobalTaskQueue& operator=(const GlobalTaskQueue&) = delete;  // non-copyassignable;
    GlobalTaskQueue(GlobalTaskQueue&&) = delete;                  // non-movable;
    GlobalTaskQueue& operator=(GlobalTaskQueue&&) = delete;       // non-moveassignable;

    void push_task(ValueType task);
    std::optional<ValueType> try_extract_task();

    // docking is process of joining a tasks batch to the global_queue,
    // which is wrapped in an IntrusiveList:
    void dock_tasks(vvv::IntrusiveList<TaskT>&& list);

    // undocking is reverse operation to the `dock_list` - serve to pick up
    // tasks batch from the global queue, which wrapped in an IntrusiveList:
    std::optional<vvv::IntrusiveList<TaskT>> undock_tasks(size_t count);
};

}  // namespace wr
