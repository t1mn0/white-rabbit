#pragma once

#include "../tasks/task.hpp"

namespace wr {

struct IScheduler {
  public:  // member-functions:
    virtual void submit(TaskBase*) = 0;

  protected:  // member-functions:
    // The `protected` access specifier makes it impossible
    // to delete object of derived class via pointer to the base class;
    ~IScheduler() = default;
};  // class IScheduler

}  // namespace wr
