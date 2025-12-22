#pragma once


#include "shared_state.hpp"
#include <memory>

namespace wr::queues {

template <size_t Capacity>
class StealHandle {

  private:
    std::shared_ptr<SharedState<Capacity>> state_;
};
};  // namespace wr::queues
