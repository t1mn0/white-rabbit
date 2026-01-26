#pragma once

#include "ring_buffer.hpp"
#include "utils.hpp"

#include <cstddef>
#include <cstdint>

namespace wr::queues {

template <task::Task TaskT, size_t Capacity>
    requires utils::check::IsPowerOfTwo<Capacity>
struct SharedState {
  private:  // fields:
    RingBuffer<TaskT, Capacity> tasks_buffer_;
    alignas(utils::constants::CACHE_LINE_SIZE) std::atomic<uint64_t> top_{0};
    alignas(utils::constants::CACHE_LINE_SIZE) std::atomic<uint64_t> bottom_{0};
};

};  // namespace wr::queues
