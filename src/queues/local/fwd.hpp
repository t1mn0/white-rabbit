#pragma once

#include "../../tasks/concept.hpp"
#include "utils.hpp"

namespace wr::queues {

template <task::Task TaskType, size_t Capacity>
    requires utils::constants::check::IsPowerOfTwo<Capacity>
class WorkStealingQueue;


template <task::Task TaskType, size_t Capacity>
class StealHandle;

}  // namespace wr::queues
