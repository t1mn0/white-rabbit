#pragma once

#include <vvv/list.hpp>

#include "../../tasks/concept.hpp"

#include <condition_variable>
#include <mutex>
#include <optional>

namespace wr {

// 1. Unbounded
// 2. Blocking
// 3. MP-MC
template <task::Task TaskT>
class GlobalQueue {
  public:  // member-types:
    using TaskPtr = TaskT*;
    using Batch = vvv::IntrusiveList<TaskT>;

  private:                      // fields:
    mutable std::mutex mutex_;  // `mutable`
    std::condition_variable not_empty_;
    vvv::IntrusiveList<TaskT> buffer_;

  public:  // member-functions:
    GlobalQueue() = default;
    ~GlobalQueue() = default;
    GlobalQueue(const GlobalQueue&) = delete;             // non-copyable;
    GlobalQueue& operator=(const GlobalQueue&) = delete;  // non-copyassignable;
    GlobalQueue(GlobalQueue&&) = delete;                  // non-movable;
    GlobalQueue& operator=(GlobalQueue&&) = delete;       // non-moveassignable;

    // -------------------- Producer API --------------------

    // Pushes a single task to the back of the queue
    void push(TaskPtr task) noexcept;

    // Docks a batch of tasks to GlobalQueue.
    // O(1) complexity thanks to vvv::IntrusiveList::Append
    void push_batch(Batch&& batch) noexcept;

    // -------------------- Consumer API --------------------

    // Tries to extract one task. Returns std::nullopt if empty
    std::optional<TaskPtr> try_pop() noexcept;

    // Tries to extract a batch of tasks up to max_count.
    std::optional<Batch> try_pop_batch(size_t max_count) noexcept;

  private:  // member-functions:
    // `.empty()` needed not for the internal logic of shifting tasks, but for
    // external monitoring of the system status and for the logic of scheduler's shutdown:
    bool is_empty() const noexcept;

  private:  // friends declaration:
            // . . .
};

}  // namespace wr

#include "global_queue.tpp"
