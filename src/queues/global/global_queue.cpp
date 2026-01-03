#pragma once

#include "global_queue.hpp"

namespace wr {

void GlobalTaskQueue::push_task(TaskBase* task) {
    std::lock_guard lock(mutex_);

    // An argument of the `IntrusiveListNode*` type is expected in
    // `IntrusiveList<TaskBase>.PushBack(Node* node)`; But since
    // `TaskBase` is a special case of `IntrusiveListNode`, this operation is acceptable:
    buffer_.PushBack(task);  // implicit upcast: TaskBase* -> IntrusiveListNode*;

    not_empty_.notify_one();
}

std::optional<TaskBase*> GlobalTaskQueue::try_extract_task() {
    std::lock_guard lock(mutex_);

    if (buffer_.IsEmpty()) {
        return std::nullopt;
    }

    TaskBase* task = buffer_.PopFrontNonEmpty();  // implicit downcast: IntrusiveListNode* -> TaskBase*;
    return task;
}

void GlobalTaskQueue::dock_tasks(vvv::IntrusiveList<TaskBase>&& list) {
    if (list.IsEmpty()) {
        return;
    }

    std::lock_guard lock(mutex_);

    // [TODO ?]
    // We can implement in our IntrusiveList impl:
    // .splice(
    //      vvv::IntrusiveList<TaskBase> list,
    //      vvv::IntrusiveListIterator<TaskBase> previous_iter = this->end()
    // );
    // And write:
    //   buffer_.splice(list);
    // Instead of this loop:
    while (!list.IsEmpty()) {
        TaskBase* task = list.PopFrontNonEmpty();  // implicit downcast: IntrusiveListNode* -> TaskBase*;
        buffer_.PushBack(task);                    // implicit upcast: TaskBase* -> IntrusiveListNode*;
    }

    not_empty_.notify_one();
}

std::optional<vvv::IntrusiveList<TaskBase>> GlobalTaskQueue::undock_tasks(size_t count) {
    std::lock_guard lock(mutex_);

    if (buffer_.IsEmpty() || count == 0) {
        return std::nullopt;
    }

    vvv::IntrusiveList<TaskBase> result;

    size_t actual_count = 0;

    while (!buffer_.IsEmpty() && actual_count < count) {
        TaskBase* task = buffer_.PopFrontNonEmpty();  // implicit downcast: IntrusiveListNode* -> TaskBase*;
        result.PushBack(task);                        // implicit upcast: TaskBase* -> IntrusiveListNode*;
        ++actual_count;
    }

    return result;
}

}  // namespace wr
