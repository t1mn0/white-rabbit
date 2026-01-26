#pragma once

#include "loot.hpp"
#include "shared_state.hpp"

namespace wr::queues {

template <task::Task TaskT, size_t Capacity>
class StealHandle {
  private:  // fields:
    SharedState<TaskT, Capacity>* state_;

  public:  // member-functions:
    explicit StealHandle(SharedState<TaskT, Capacity>* state) : state_(state) {}

    queues::Loot<TaskT> steal();  // !TODO!
};

};  // namespace wr::queues
