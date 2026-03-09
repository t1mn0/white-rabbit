#pragma once

#include "fwd.hpp"
#include "loot.hpp"
#include "shared_state.hpp"

namespace wr::queues {

/**
 * @brief StealHandle is an interface for stealers.
 *
 * @section LIFETIME
 *
 *  >> StealHandle must not outlive a local queue
 */

template <task::Task TaskType, size_t Capacity>
class StealHandle {
  private:  // data members:
    /* We're holding ptr tthe other worker's shared state. It's like ticket or pass for the stealing */
    SharedState<TaskType, Capacity>* state_;

  public:  // friendship declaration:
    friend class WorkStealingQueue<TaskType, Capacity>;

  public:  // member functions:
    explicit StealHandle(SharedState<TaskType, Capacity>* state) : state_(state) {}

    [[nodiscard]]
    /* !! TODO !! */ Loot<TaskType> steal() noexcept;

    [[nodiscard]]
    /* !! TODO !! */ Loot<TaskType> steal_batch_and_pop(WorkStealingQueue<TaskType, Capacity>& dest) noexcept;

    [[nodiscard]]
    bool is_empty() const noexcept;

    StealHandle(const StealHandle& other) = default;
    StealHandle(StealHandle&&) = default;
    StealHandle& operator=(const StealHandle&) = default;
    StealHandle& operator=(StealHandle&&) = default;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

template <task::Task TaskType, size_t Capacity>
Loot<TaskType> StealHandle<TaskType, Capacity>::steal() noexcept {
    uint64_t top = state_->top_.load(std::memory_order_acquire /* ??? */);

    std::atomic_thread_fence(std::memory_order_seq_cst /* ??? */);

    uint64_t bottom = state_->bottom_.load(std::memory_order_acquire /* ??? */);

    if (top >= bottom) {
        return Loot<TaskType>::Empty();
    }

    TaskType* task = state_->load_task(top /*, ??? */);

    if (!state_->try_increment_top(top /*, ??? */)) {
        return Loot<TaskType>::Retry();
    }

    return Loot<TaskType>::Success(task);
}

template <task::Task TaskType, size_t Capacity>
bool StealHandle<TaskType, Capacity>::is_empty() const noexcept {
    return state_->top_.load(std::memory_order_relaxed) >=
           state_->bottom_.load(std::memory_order_relaxed);
}

};  // namespace wr::queues
