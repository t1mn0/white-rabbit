#pragma once


#include "shared_state.hpp"
#include <memory>

namespace wr::queues {

template <task::Task TaskT, size_t Capacity>
class StealHandle {
  private:
    std::shared_ptr<SharedState<TaskT, Capacity>> state_;
};
};  // namespace wr::queues
