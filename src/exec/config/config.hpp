#pragma once

#include <cstddef>
#include <cstdint>

namespace wr::config {

struct DefaultConfig {
    static constexpr size_t kLocalQueueCapacity = 8192;
    static constexpr size_t kMaxLifoStreak = 23;
    static constexpr uint64_t kFairnessPeriod = 61;
};

struct TinyConfig {
    static constexpr std::size_t kLocalQueueCapacity = 256;
    static constexpr int kMaxLifoStreak = 2;
    static constexpr std::uint64_t kFairnessPeriod = 31;
};

}  // namespace wr::config
