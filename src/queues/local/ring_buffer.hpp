#pragma once

#include "../../tasks/task.hpp"
#include "utils.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace wr::queues {

template <size_t Capacity>
    requires PowerOfTwo<Capacity>
class RingBuffer {
  public:
    using ValueType = TaskBase*;

    ValueType load(uint64_t index) const;

    void store(uint64_t index, ValueType value);

  private:
    static constexpr size_t mask = Capacity - 1;

    static constexpr size_t to_local_index(uint64_t global);

    std::array<std::atomic<ValueType>, Capacity> slots_;
};

};  // namespace wr::queues
