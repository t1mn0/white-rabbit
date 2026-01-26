#pragma once

#include "../../tasks/concept.hpp"
#include "shared_state.hpp"
#include "steal_handle.hpp"
#include <memory>

#ifdef __cpp_lib_hardware_interference_size
inline constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
inline constexpr std::size_t CACHE_LINE_SIZE = 64;
#endif

namespace wr {

// Forward-decl in 'queues' sub-namespace:
namespace queues {
template <task::Task TaskT, size_t Capacity>
class StealHandle;
}  // namespace queues

/**
 * Work-stealing spmc lock-free local queue
 */
template <task::Task TaskT, size_t Capacity = 8196>
    requires IsPowerOfTwo<Capacity>
class WorkStealingQueue {
  public:
    WorkStealingQueue();
    ~WorkStealingQueue();

    WorkStealingQueue(const WorkStealingQueue&) = delete;
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;


    bool try_push(TaskT* item);

    TaskT* pop();

    int pop_batch(TaskT** output_buffer, size_t max_count);

    queues::StealHandle<TaskT, Capacity> stealer();

  private:
    std::shared_ptr<queues::SharedState<TaskT, Capacity>> shared_state_;
};

}  // namespace wr
