#pragma once

template <typename T>
class IntrusiveListNode {
    // . . .
};

namespace wr {

struct ITask {
  public:
    virtual void run() = 0;  // run exec unit;
    virtual ~ITask() = default;
};

struct TaskBase : ITask, IntrusiveListNode<TaskBase> {
};

}  // namespace wr
