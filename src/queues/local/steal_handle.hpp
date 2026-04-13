#pragma once

#include "fwd.hpp"
#include "loot.hpp"
#include "shared_state.hpp"
#include "tasks/concept.hpp"
#include <atomic>

namespace wr::queues {

/**
 * @brief StealHandle is an interface for stealers.
 *
 * @section Lifetime
 *
 * >> StealHandle must not outlive a local queue
 */

template <task::Task TaskType, size_t Capacity>
class StealHandle {
  private:
    /* *---*---*---*---*---*---*---*---*---* */

    /*
     * We're holding ptr tthe other worker's shared state.
     * It's like ticket or pass for the stealing
     */
    SharedState<TaskType, Capacity>* state_;

    /* *---*---*---*---*---*---*---*---*---* */

  public:  // friendship declaration:
    friend class WorkStealingQueue<TaskType, Capacity>;

  public:  // member functions:
    explicit StealHandle(SharedState<TaskType, Capacity>* state)
        : state_(state) {}

    [[nodiscard]]
    auto steal_task() noexcept -> Loot<TaskType>;

    [[nodiscard]]
    Loot<TaskType> steal_batch_and_pop(WorkStealingQueue<TaskType, Capacity>& dest) noexcept;

    [[nodiscard]]
    bool empty() const noexcept;

    StealHandle(const StealHandle& other) = default;
    StealHandle(StealHandle&&) = default;
    StealHandle& operator=(const StealHandle&) = default;
    StealHandle& operator=(StealHandle&&) = default;
};

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---* */

/**
 * TODO : figure out about mmry orders here...
 * [to prove in terms of partial orders]
 */
template <task::Task TaskType, size_t Capacity>
auto StealHandle<TaskType, Capacity>::steal_task() noexcept -> Loot<TaskType> {
    uint64_t top = state_->load_top_idx(std::memory_order_seq_cst /* ??? */);

    std::atomic_thread_fence(std::memory_order_seq_cst /* ??? */);

    uint64_t btm = state_->load_bottom_idx(std::memory_order_seq_cst /* ??? */);

    if (top >= btm) {
        return Loot<TaskType>::Empty();
    }

    TaskType* task = state_->load_task(top /*, ??? */);

    if (!state_->try_increment_top(top /*, ??? */)) {
        return Loot<TaskType>::Retry();
    }

    return Loot<TaskType>::Success(task);
}

template <task::Task TaskType, size_t Capacity>
bool StealHandle<TaskType, Capacity>::empty() const noexcept {
    ///
    return state_->load_top_idx() >= state_->load_bottom_idx();
    ///
}
};  // namespace wr::queues
