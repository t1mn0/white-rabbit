#pragma once

#include "../../tasks/concept.hpp"
#include "utils.hpp"

#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>

namespace wr::queues {


template <task::Task TaskT, size_t Capacity>
    requires utils::check::IsPowerOfTwo<Capacity>
class RingBuffer {
  public:  // member-types:
    using ValueType = TaskT*;

  private:  // fields:
    static constexpr size_t mask_ = Capacity - 1;
    std::array<std::atomic<ValueType>, Capacity> slots_;

  public:  // member-functions:
    ValueType load(uint64_t index) const;
    void store(uint64_t index, ValueType val);

    size_t capacity() const { return Capacity; }
};

/* ------------------------------------------------------------------- */

template <task::Task TaskT, size_t Capacity>
    requires utils::check::IsPowerOfTwo<Capacity>
RingBuffer<TaskT, Capacity>::ValueType RingBuffer<TaskT, Capacity>::load(uint64_t index) const {
    return slots_[index & mask_].load();
}

template <task::Task TaskT, size_t Capacity>
    requires utils::check::IsPowerOfTwo<Capacity>
void RingBuffer<TaskT, Capacity>::store(uint64_t index, ValueType val) {
    slots_[index & mask_].store(val);
}

};  // namespace wr::queues
