#pragma once

#include "../../tasks/concept.hpp"
#include "shared_state.hpp"
#include "steal_handle.hpp"

#include <optional>

namespace wr {

// 1. Bounded by template param
// 2. Lock-free
// 3. SP-MC
template <task::Task TaskT, size_t Capacity = 8196>
    requires utils::check::IsPowerOfTwo<Capacity>
class WorkStealingQueue {
  private:  // fields:
    queues::SharedState<TaskT, Capacity> shared_state_;

  public:  // member-functions:
    WorkStealingQueue();
    ~WorkStealingQueue() = default;

    WorkStealingQueue(const WorkStealingQueue&) = delete;             // non-copyable
    WorkStealingQueue(WorkStealingQueue&&) = delete;                  // non-movable
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;  // non-copyassignable
    WorkStealingQueue& operator=(WorkStealingQueue&&) = delete;       // non-moveassignable

    // -------------------- Producer API --------------------
    bool try_push_task(TaskT* item);                                              // !TODO!
    TaskT* extract_task();                                                        // !TODO!
    std::optional<vvv::IntrusiveList<TaskT>> try_undock_tasks(size_t max_count);  // !TODO!

    // -------------------- Consumer API --------------------
    queues::StealHandle<TaskT, Capacity> stealer() {
        return queues::StealHandle<TaskT, Capacity>(&shared_state_);
    }
};

}  // namespace wr
