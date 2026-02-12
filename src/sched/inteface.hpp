#pragma once

#include "../tasks/concept.hpp"

namespace wr::sched::detail {

template <task::Task TaskT>
struct IScheduler {
  public:  // member-functions:
    virtual void submit(TaskT*) = 0;

  protected:  // member-functions:
    // The `protected` access specifier makes it impossible
    // to delete object of derived class via pointer to the base class;
    ~IScheduler() = default;
};  // class IScheduler

}  // namespace wr::sched::detail
