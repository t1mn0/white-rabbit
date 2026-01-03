#pragma once

#include <vvv/list.hpp>

namespace wr {

struct ITask {
  public:  // member-functions:
    virtual void run() = 0;

  protected:  // member-functions:
    // Virtual destructor is not used, since ITask is not supposed
    // to be used instead of TaskBase or other derived classes in
    // this chain of inheritance;
    // Also, the `protected` access specifier makes it impossible
    // to delete object of derived class via pointer to the base class;
    ~ITask() = default;
};

struct TaskBase : public ITask, vvv::IntrusiveListNode<TaskBase> {};

}  // namespace wr
