#include "global_queue.hpp"

namespace wr {

template <task::Task TaskT>
void GlobalTaskQueue<TaskT>::push_task(GlobalTaskQueue<TaskT>::ValueType task) {
    std::lock_guard lock(mutex_);

    // An argument of the `IntrusiveListNode*` type is expected in
    // `IntrusiveList<TaskT>.PushBack(Node* node)`; But since
    // `TaskT` is a special case of `IntrusiveListNode`, this operation is acceptable:
    buffer_.PushBack(task);  // implicit upcast: TaskT* -> IntrusiveListNode*;

    not_empty_.notify_one();
}

template <task::Task TaskT>
std::optional<typename GlobalTaskQueue<TaskT>::ValueType> GlobalTaskQueue<TaskT>::try_extract_task() {
    std::lock_guard lock(mutex_);

    if (buffer_.IsEmpty()) {
        return std::nullopt;
    }

    ValueType task = buffer_.PopFrontNonEmpty();  // implicit downcast: IntrusiveListNode* -> TaskT*;
    return task;
}

template <task::Task TaskT>
void GlobalTaskQueue<TaskT>::dock_tasks(vvv::IntrusiveList<TaskT>&& list) {
    if (list.IsEmpty()) {
        return;
    }

    std::lock_guard lock(mutex_);

    // [TODO ?]
    // We can implement in our IntrusiveList impl:
    // .splice(
    //      vvv::IntrusiveList<TaskT> list,
    //      vvv::IntrusiveListIterator<TaskBase> previous_iter = this->end()
    // );
    // And write:
    //   buffer_.splice(list);
    // Instead of this loop:
    while (!list.IsEmpty()) {
        ValueType task = list.PopFrontNonEmpty();  // implicit downcast: IntrusiveListNode* -> TaskBase*;
        buffer_.PushBack(task);                    // implicit upcast: TaskBase* -> IntrusiveListNode*;
    }

    not_empty_.notify_one();
}

template <task::Task TaskT>
std::optional<vvv::IntrusiveList<TaskT>> GlobalTaskQueue<TaskT>::try_undock_tasks(size_t max_count) {
    std::lock_guard lock(mutex_);

    if (buffer_.IsEmpty() || max_count == 0) {
        return std::nullopt;
    }

    vvv::IntrusiveList<TaskT> result;

    size_t actual_count = 0;

    while (!buffer_.IsEmpty() && actual_count < max_count) {
        ValueType task = buffer_.PopFrontNonEmpty();  // implicit downcast: IntrusiveListNode* -> TaskT*;
        result.PushBack(task);                        // implicit upcast: TaskT* -> IntrusiveListNode*;
        ++actual_count;
    }

    return result;
}

}  // namespace wr
