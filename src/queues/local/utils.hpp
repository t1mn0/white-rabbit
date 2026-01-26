#pragma once

#include <cstddef>

// If any of these functions or constants are required in other pseudo-modules
// in the future, then `utils`-dir will be moved a few folders higher and make it common;

namespace wr::utils {

namespace constants {

#ifdef __cpp_lib_hardware_interference_size
inline constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
inline constexpr std::size_t CACHE_LINE_SIZE = 64;
#endif

}  // namespace constants

namespace check {

constexpr bool is_power_of_two(size_t n) noexcept {
    return n > 0 && (n & (n - 1)) == 0;
}

template <size_t N>
concept IsPowerOfTwo = is_power_of_two(N);

}  // namespace check

}  // namespace wr::utils
