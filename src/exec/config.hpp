#pragma once

#include "../queues/local/utils.hpp"
#include <concepts>
#include <cstddef>
#include <cstdint>

namespace wr::config {

struct DefaultConfig {
    static constexpr size_t kLocalQueueCapacity = 8192;
    static constexpr size_t kMaxLifoStreak = 23;
    static constexpr uint64_t kFairnessPeriod = 61;
};

template <typename C>
concept ExecutionConfig = requires {
    { C::kLocalQueueCapacity } -> std::convertible_to<size_t>;
    { C::kMaxLifoStreak } -> std::convertible_to<size_t>;
    { C::kFairnessPeriod } -> std::convertible_to<size_t>;

    requires utils::constants::check::is_power_of_two(C::kLocalQueueCapacity);
};

struct TinyConfig {
    static constexpr std::size_t kLocalQueueCapacity = 256;
    static constexpr int kMaxLifoStreak = 2;
    static constexpr std::uint64_t kFairnessPeriod = 31;
};

}  // namespace wr::config
