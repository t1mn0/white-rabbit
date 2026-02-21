#pragma once

#include "../../queues/local/utils.hpp"

#include <concepts>

namespace wr::config {

template <typename C>
concept ExecutionConfig = requires {
    { C::kLocalQueueCapacity } -> std::convertible_to<size_t>;
    { C::kMaxLifoStreak } -> std::convertible_to<size_t>;
    { C::kFairnessPeriod } -> std::convertible_to<size_t>;

    requires utils::constants::check::is_power_of_two(C::kLocalQueueCapacity);
};

}  // namespace wr::config
