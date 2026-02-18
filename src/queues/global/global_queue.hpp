#pragma once

#include <vvv/list.hpp>

#include "../../tasks/concept.hpp"

#include <condition_variable>
#include <mutex>
#include <optional>

namespace wr::queues {

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

/* ---------------------------------- IMPLEMENTATION ---------------------------------- */

template <task::Task TaskT>
void GlobalQueue<TaskT>::push(TaskPtr task) noexcept {
    {
        std::lock_guard lock(mutex_);

        // An argument of the `IntrusiveListNode*` type is expected in
        // `IntrusiveList<TaskT>.PushBack(Node* node)`; But since `TaskT`
        // is a special case of `IntrusiveListNode`, this operation is acceptable:
        buffer_.PushBack(task);  // implicit upcast: TaskT* -> IntrusiveListNode*;
    }

    // mutex is no longer valid here, the notified thread can quickly pick up the critical section:
    not_empty_.notify_one();
}

template <task::Task TaskT>
std::optional<typename GlobalQueue<TaskT>::TaskPtr> GlobalQueue<TaskT>::try_pop() noexcept {
    std::lock_guard lock(mutex_);

    if (buffer_.IsEmpty()) {
        return std::nullopt;
    }

    return buffer_.PopFrontNonEmpty();  // implicit downcast: IntrusiveListNode* -> TaskT*;;
}

template <task::Task TaskT>
void GlobalQueue<TaskT>::push_batch(Batch&& batch) noexcept {
    if (batch.IsEmpty()) {
        return;
    }

    {
        std::lock_guard lock(mutex_);
        buffer_.Append(batch);  // Complexity: O(1)
    }

    not_empty_.notify_one();
}

template <task::Task TaskT>
auto GlobalQueue<TaskT>::try_pop_batch(size_t max_count) noexcept
    -> std::optional<typename GlobalQueue<TaskT>::Batch> {

    if (max_count == 0) {
        return std::nullopt;
    }

    std::lock_guard lock(mutex_);
    if (buffer_.IsEmpty()) {
        return std::nullopt;
    }

    Batch result;
    size_t actual_count = 0;
    while (!buffer_.IsEmpty() && actual_count < max_count) {
        result.PushBack(buffer_.PopFrontNonEmpty());
        ++actual_count;
    }

    return result;
}

template <task::Task TaskT>
bool GlobalQueue<TaskT>::is_empty() const noexcept {
    std::lock_guard lock(mutex_);
    return buffer_.IsEmpty();
}


}  // namespace wr::queues
