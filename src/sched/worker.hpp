#pragma once

#include "scheduler.hpp"
#include <memory>

namespace wr {

class Worker {
  public:
  private:
    std::unique_ptr<Scheduler> scheduler_;
    size_t tick_ = 0;
    const size_t index_;

    TaskBase* warm_slot_{nullptr}; /* LIFO */
};

};  // namespace wr
