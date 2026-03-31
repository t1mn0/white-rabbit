#pragma once

#include <algorithm>
#include <mutex>
#include <optional>

#include <ntrusive/intrusive.hpp>

#include "../../tasks/concept.hpp"

namespace wr::queues {

// >> Unboundeds
// >> Blocking
// >> MP-MC
template <task::Task TaskT>
class GlobalQueue {
  public:
    /* *---*---*---*---*---*---* */

    using TaskPtr = TaskT*;
    using Batch = IntrusiveList<TaskT>;

  private:
    /* *---*---*---*---*---*---* */

    IntrusiveList<TaskT> buffer_;

    /* Point of contention:
     * @note mutable since it is used in the constant method .empty()
     */
    mutable std::mutex mutex_;

    /* *---*---*---*---*---*---* */

  public:  // member functions:
    GlobalQueue() = default;
    ~GlobalQueue() = default;
    GlobalQueue(const GlobalQueue&) = delete;             // non-copyable;
    GlobalQueue& operator=(const GlobalQueue&) = delete;  // non-copyassignable;
    GlobalQueue(GlobalQueue&&) = delete;                  // non-movable;
    GlobalQueue& operator=(GlobalQueue&&) = delete;       // non-moveassignable;

    // Pushes a single task to the back of the queue
    void push(TaskPtr task) noexcept;

    // Docks a batch of tasks to GlobalQueue.
    // O(1) complexity thanks to IntrusiveList::splice
    void push_batch(Batch&& batch) noexcept;

    // Tries to extract one task. Returns std::nullopt if empty
    auto try_pop() noexcept -> std::optional<TaskPtr>;

    // Tries to extract a batch of tasks up to max_count.
    // O(max_count) complexity
    auto try_pop_batch(size_t max_count) noexcept -> std::optional<Batch>;

  private:  // member functions:
    // `empty()` needed not for the internal logic of shifting tasks, but for
    // external monitoring of the system status and for the logic of scheduler's shutdown:
    auto empty() const noexcept -> bool;

  private:  // TODO: friends declaration for .empty() users:
            // . . .
};

/* *---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*---*--- */

template <task::Task TaskT>
void GlobalQueue<TaskT>::push(TaskPtr task) noexcept {
    std::lock_guard lock(mutex_);
    buffer_.push_back(*task);
}

template <task::Task TaskT>
auto GlobalQueue<TaskT>::try_pop() noexcept -> std::optional<TaskPtr> {
    std::lock_guard lock(mutex_);

    if (buffer_.empty()) {
        return std::nullopt;
    }

    return buffer_.try_pop_front();
}

template <task::Task TaskT>
void GlobalQueue<TaskT>::push_batch(Batch&& batch) noexcept {
    if (batch.empty()) {
        return;
    }

    {
        std::lock_guard lock(mutex_);
        buffer_.splice(buffer_.end(), batch);  // O(1)
    }
}

template <task::Task TaskT>
auto GlobalQueue<TaskT>::try_pop_batch(size_t max_count) noexcept -> std::optional<Batch> {
    if (max_count == 0) {
        return std::nullopt;
    }

    std::lock_guard lock(mutex_);
    if (buffer_.empty()) {
        return std::nullopt;
    }

    Batch result;
    size_t actual_count = buffer_.extract_front(result, max_count);

    if (actual_count == 0) {
        return std::nullopt;
    }

    return std::make_optional(std::move(result));
}

template <task::Task TaskT>
bool GlobalQueue<TaskT>::empty() const noexcept {
    std::lock_guard lock(mutex_);
    return buffer_.empty();
}

}  // namespace wr::queues
