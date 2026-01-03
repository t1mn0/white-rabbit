#pragma once

#include <vvv/list.hpp>

namespace wr {

struct ITask {
  public:
    virtual void run() = 0;  // run exec unit;
    virtual ~ITask() = default;
};

struct TaskBase : ITask, vvv::IntrusiveListNode<TaskBase> {
};

}  // namespace wr
