#pragma once

#include "../../tasks/concept.hpp"
#include "shared_state.hpp"
#include "steal_handle.hpp"
#include "utils.hpp"
#include <cstddef>
#include <optional>
#include <vvv/list.hpp>

namespace wr::queues {

// >> Bounded
// >> Lock-free
// >> SP-MC
template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class WorkStealingQueue {
  public:  // member-functions:
    WorkStealingQueue();
    ~WorkStealingQueue() = default;

    WorkStealingQueue(const WorkStealingQueue&) = delete;             // non-copyable
    WorkStealingQueue(WorkStealingQueue&&) = delete;                  // non-movable
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;  // non-copyassignable
    WorkStealingQueue& operator=(WorkStealingQueue&&) = delete;       // non-moveassignable

    /*  -------------------- Producer API -------------------- */

    /*
     * @brief Push task at the bottom, returns False if queue is full.
     */
    bool try_push(TaskType* item) noexcept;

    /* !! TODO !! */ std::optional<vvv::IntrusiveList<TaskType>> try_undock_tasks(size_t max_count);


    /*  -------------------- Consumer API -------------------- */
    /*
     * @brief Try pop task from the bottom, returns nullopt if empty.
     */
    std::optional<TaskType*> try_pop() noexcept;

    template <typename Container>
    int pop_batch(Container& where, size_t max_count);

    /*
     * @brief Offload half of the local queue to the global one if the local queue turns out to be full
     * when attempting a push operation.
     *
     * @return List of offloaded tasks.
     */
    vvv::IntrusiveList<TaskType> offload_half() noexcept;

    [[nodiscard]]
    /* !! TODO !! */ StealHandle<TaskType, Capacity> create_stealer() noexcept;

  private:  // fields:
    SharedState<TaskType, Capacity> state_;

    friend class StealHandle<TaskType, Capacity>;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
bool WorkStealingQueue<TaskType, Capacity>::try_push(TaskType* task) noexcept {
    /*
     * since stealers are never touch the bottom of the buffer
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

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskType, Capacity>::try_pop() noexcept -> std::optional<TaskType*> {
    auto top = state
}


}  // namespace wr::queues
