#pragma once

#include <cstddef>

constexpr bool is_power_of_two(size_t n) noexcept {
    return n > 0 && (n & (n - 1)) == 0;
}

template <size_t N>
concept IsPowerOfTwo = is_power_of_two(N);
