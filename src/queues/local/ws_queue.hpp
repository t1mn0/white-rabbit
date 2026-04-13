#pragma once

#include <atomic>
#include <cstddef>
#include <ntrusive/intrusive.hpp>
#include <optional>

#include "../../tasks/concept.hpp"
#include "shared_state.hpp"
#include "steal_handle.hpp"
#include "utils/constants.hpp"

namespace wr::queues {

// >> Bounded
// >> Lock-free
// >> SP-MC
template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class WorkStealingQueue {
  public:
    using TaskPtr = TaskType*;
    using Batch = IntrusiveList<TaskType>;

  private:
    /* *---*---*---*---*---*---*---*---*---* */

    SharedState<TaskType, Capacity> state_;

    /* *---*---*---*---*---*---*---*---*---* */

  public:
    friend class StealHandle<TaskType, Capacity>;

  public:  // member functions:
    WorkStealingQueue() = default;
    ~WorkStealingQueue() = default;

    WorkStealingQueue(const WorkStealingQueue&) = delete;             // non-copyable
    WorkStealingQueue(WorkStealingQueue&&) = delete;                  // non-movable
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;  // non-copyassignable
    WorkStealingQueue& operator=(WorkStealingQueue&&) = delete;       // non-moveassignable

    /*
     * @brief Push task at the bottom, returns False if queue is full.
     */
    auto try_push(TaskPtr item) noexcept -> bool;

    /*
     * @brief Try pop task from the bottom, returns nullopt if empty.
     */
    auto try_pop() noexcept -> std::optional<TaskPtr>;


    /*
     * @brief Offload half of all tasks from the local queue to the global one if
     *        the local queue turns out to be full when attempting a push operation.
     *
     * @return List of offloaded tasks.
     */
    auto offload_half() noexcept -> std::optional<Batch>;

    [[nodiscard]]
    auto create_stealer() noexcept -> StealHandle<TaskType, Capacity>;

    [[nodiscard]]
    auto inner_state() const noexcept -> const SharedState<TaskType, Capacity>&;
};

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*--- */

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskType, Capacity>::try_push(TaskPtr task) noexcept -> bool {
    /*
     * since stealers are never touch the bottom of the buffer
     * => only worker (producer) works with it on a separate cache-line
     * => relaxed mo
     */
    auto bt = state_.load_bottom_idx(std::memory_order::relaxed);

    auto top = state_.load_top_idx();

    /* capacity cheeeck.. */
    if (bt - top >= Capacity) {
        return false;
    }
    state_.store_task(bt, task);

    /* maybe fence right here..?
     * TODO : figure out what mo to use right here!!?
     */
    std::atomic_thread_fence(std::memory_order::seq_cst /* ??? */);

    state_.store_bottom_idx(++bt);

    return true;
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskType, Capacity>::try_pop() noexcept -> std::optional<TaskPtr> {
    /* relaxedd mo here for the same reason [worker is the only one that has access tthe bottom]  */
    auto bt = state_.load_bottom_idx(std::memory_order::relaxed);
    auto top = state_.load_top_idx();

    /* queue is empty... */
    if (top == bt) {
        return std::nullopt;
    }

    bt -= 1;
    state_.store_bottom_idx(bt);

    /* TODO : [to clarify and proof these guarantees in terms of partial orders]
     * TODO : [test with Twist simulations this case]
     */
    std::atomic_thread_fence(std::memory_order::seq_cst);

    top = state_.load_top_idx();

    if (top < bt) {
        /* => top <= bottom */
        return state_.load_task(bt);
    }

    /* TODO : [refactoring] : separate race() function */

    /* => otherwise, top == bottom
     * => race with stealers for the last element
     */
    if (state_.try_increment_top(top)) {
        /* we won the race */
        state_.store_bottom_idx(bt + 1);
        return state_.load_task(bt);

    } else {
        /* stealer won... */
        /* cancellation... */
        state_.store_bottom_idx(bt + 1);
        return std::nullopt;
    }
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskType, Capacity>::create_stealer() noexcept -> StealHandle<TaskType, Capacity> {
    ///
    return StealHandle<TaskType, Capacity>(&state_);
    ///
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskType, Capacity>::offload_half() noexcept -> std::optional<Batch> {
    IntrusiveList<TaskType> batch;

    auto bt = state_.load_bottom_idx();
    auto top = state_.load_top_idx();

    auto size = bt - top;

    if (size <= 1) {
        return std::nullopt;
    }

    size_t offload_count = size / 2;

    if (!state_.try_increment_top_by(top, offload_count)) {
        return std::nullopt;
    }

    /* [top; top + count) */
    for (auto i = top; i < top + offload_count; ++i) {
        auto task = state_.load_task(i);
        batch.push_back(*task);
    }

    return batch;
}

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskType, Capacity>::inner_state() const noexcept
    -> const SharedState<TaskType, Capacity>& {
    ///
    return state_;
    ///
}

}  // namespace wr::queues
