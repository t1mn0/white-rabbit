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
template <task::Task TaskT, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class WorkStealingQueue {
  public:  // nested types:
    using TaskPtr = TaskT*;
    using Batch = IntrusiveList<TaskT>;

  private:  // data members:
    SharedState<TaskT, Capacity> state_;

  public:  // friendship declaration:
    friend class StealHandle<TaskT, Capacity>;

  public:  // member functions:
    WorkStealingQueue() = default;
    ~WorkStealingQueue() = default;

    WorkStealingQueue(const WorkStealingQueue&) = delete;             // non-copyable
    WorkStealingQueue(WorkStealingQueue&&) = delete;                  // non-movable
    WorkStealingQueue& operator=(const WorkStealingQueue&) = delete;  // non-copyassignable
    WorkStealingQueue& operator=(WorkStealingQueue&&) = delete;       // non-moveassignable

    /*  -------------------- Producer API -------------------- */

    /*
     * @brief Push task at the bottom, returns False if queue is full.
     */
    auto try_push(TaskPtr item) noexcept -> bool;

    /*  -------------------- Consumer API -------------------- */
    /*
     * @brief Try pop task from the bottom, returns nullopt if empty.
     */
    auto try_pop() noexcept -> std::optional<TaskPtr>;


    /*
     * @brief Offload half of all tasks from the local queue to the global one if the local queue turns out to
     * be full when attempting a push operation.
     *
     * @return List of offloaded tasks.
     */
    auto offload_half() noexcept -> std::optional<Batch>;

    [[nodiscard]]
    auto create_stealer() noexcept -> StealHandle<TaskT, Capacity>;
};

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

template <task::Task TaskT, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskT, Capacity>::try_push(TaskPtr task) noexcept -> bool {
    /*
     * since stealers are never touch the bottom of the buffer
     * => only worker (producer) works with it on a separate cache-line
     * => relaxed mo
     */
    auto bt = state_.load_bottom(std::memory_order::relaxed);

    auto top = state_.load_top();

    /* capacity cheeeck.. */
    if (bt - top >= Capacity) {
        return false;
    }
    state_.store_task(bt, task);

    /* maybe fence right here..? */
    std::atomic_thread_fence(std::memory_order::seq_cst /* ??? */);

    state_.store_bottom(++bt);

    return true;
}

template <task::Task TaskT, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskT, Capacity>::try_pop() noexcept -> std::optional<TaskPtr> {
    /* relaxedd mo here for the same reason [worker is the only one that has access tthe bottom]  */
    auto bt = state_.load_bottom(std::memory_order::relaxed) - 1;

    state_.store_bottom(bt);

    /* TODO : [to clarify and proof these guarantees in terms of partial orders]
     * TODO : [test with Twist simulations this case]
     */
    std::atomic_thread_fence(std::memory_order::seq_cst);

    auto top = state_.load_top();

    /* queue is empty... */
    if (top >= bt) {
        /* cancellation... */
        state_.store_bottom(++bt);
        return std::nullopt;
    }

    /* => top <= bottom */
    auto task = state_.load_task(bt);

    /* TODO : [refactoring : separate race() function ] */
    if (top < bt) {
        return task;
    }

    /* => otherwise, top == bottom
     * => race with stealers for the last element
     */
    if (state_.try_increment_top(top)) {
        /* we won the race */
        state_.store_bottom(++bt);
        return task;

    } else {
        /* stealer won... */
        state_.store_bottom(++bt);
        return std::nullopt;
    }
}

template <task::Task TaskT, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskT, Capacity>::create_stealer() noexcept -> StealHandle<TaskT, Capacity> {
    ///
    return StealHandle<TaskT, Capacity>(&state_);
    ///
}

template <task::Task TaskT, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
auto WorkStealingQueue<TaskT, Capacity>::offload_half() noexcept -> std::optional<Batch> {
    IntrusiveList<TaskT> batch;

    auto bt = state_.load_bottom();
    auto top = state_.load_top();

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

}  // namespace wr::queues
