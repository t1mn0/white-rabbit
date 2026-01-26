#pragma once

#include "ring_buffer.hpp"
#include "utils.hpp"
#include <cstddef>
#include <cstdint>

namespace wr::queues {

template <task::Task TaskT, size_t Capacity>
    requires IsPowerOfTwo<Capacity>
struct SharedState {
    RingBuffer<TaskT, Capacity> tasks_;

    std::atomic<uint64_t> top_{0};
    std::atomic<uint64_t> bottom_{0};
};

};  // namespace wr::queues
