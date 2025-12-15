#pragma once

#include "ring_buffer.hpp"
#include "utils.hpp"
#include <cstddef>
#include <cstdint>

namespace wr::queues {


template <size_t Capacity>
    requires PowerOfTwo<Capacity>
struct SharedState {
    RingBuffer<Capacity> tasks_;

    std::atomic<uint64_t> top_{0};
    std::atomic<uint64_t> bottom_{0};
};

};  // namespace wr::queues
