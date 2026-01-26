#pragma once

#include "../../tasks/concept.hpp"
#include "utils.hpp"
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace wr::queues {

template <task::Task TaskT, size_t Capacity>
    requires IsPowerOfTwo<Capacity>
class RingBuffer {
  public:
    using ValueType = TaskT*;

    ValueType load(uint64_t index) const;

    void store(uint64_t index, ValueType value);

  private:
    static constexpr size_t mask = Capacity - 1;

    static constexpr size_t to_local_index(uint64_t global);

    std::array<std::atomic<ValueType>, Capacity> slots_;
};

};  // namespace wr::queues
