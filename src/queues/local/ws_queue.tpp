#include "ws_queue.hpp"
#include <atomic>

namespace wr::queues {

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
bool WorkStealingQueue<TaskType, Capacity>::try_push(TaskType* task) noexcept {
    /*
     * since stealers are never touch the bottom_
     * => only worker (producer) works with it on a separate cache-line
     * => relaxed mo
     */
    auto bt = state_.load_bottom(std::memory_order::relaxed);

    auto top = state_.load_top();
    if (bt - top >= Capacity) {
        return false;
    }

    state_.store_task(bt, task);
    
    /* maybe fence right here..? */
    
    state_.store_bottom(++bt);

    return true;
}


}  // namespace wr::queues
