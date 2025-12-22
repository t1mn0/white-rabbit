#pragma once

#include "../../tasks/task.hpp"
#include "shared_state.hpp"
#include "steal_handle.hpp"
#include <atomic>
#include <cstddef>
#include <memory>

namespace wr {

/**
 * Work-stealing spmc lock-free local queue
 */
template <typename T = TaskBase*, size_t Capacity = 8196>
    requires IsPowerOfTwo<Capacity>
class WorkStealingQueue {
  public:
    WorkStealingQueue();
    ~WorkStealingQueue();

    WorkStealingQueue(const WorkStealingQueue&) = delete;
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;


    bool try_push(TaskBase* item);

    TaskBase* pop();

    int pop_batch(TaskBase** output_buffer, size_t max_count);

    queues::StealHandle<Capacity> stealer();

  private:
    std::shared_ptr<queues::SharedState<Capacity>> shared_state_;
};

}  // namespace wr
