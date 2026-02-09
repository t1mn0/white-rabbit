#pragma once
#include "global_queue.hpp"

namespace wr {

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

}  // namespace wr
