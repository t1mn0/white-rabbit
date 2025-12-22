#pragma once

#include <cstddef>

/* ------------------------------------------------------------------- */

#ifdef __cpp_lib_hardware_interference_size
inline constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
inline constexpr std::size_t CACHE_LINE_SIZE = 64;
#endif

/* ------------------------------------------------------------------- */

/**
 * @section POWER OF TWO REQUIREMENT
 *
 *  |> To map global index to local we have to do modulo operation.
 *
 *  But if we have a large number of requests to our circular buffer, it could be quite sloooooow...
 *
 *  But what if there was another operation that was isomorphic to the modulo, but much faster??
 *  And this operation is a logical AND. But this trick works only if we have capacity = 2^N.
 *
 *  |> [If capacity = 2^N => global % capacity = global & (mask)]
 *
 *  => The mask := [capacity - 1] cuts off all bits above the capacity
 *  and leaving only the remainder. That's exactly what modulo does.
 *
 * ---------------------------
 * EXAMPLE :
 * Capacity = 8 = 2^3 = 0b1000
 * mask = 8 - 1 = 7 = 0b0111
 *
 * => Any number AND with 0b0111 keeps only 3 lower bits:
 * global = 13 = 0b1101
 *
 *   1101
 * & 0111
 * ------
 *   0101 = 5 (13 % 8 also = 5)
 *
 * ---------------------------
 */

constexpr bool
is_power_of_two(size_t n) noexcept {
    return n > 0 && (n & (n - 1)) == 0;
}

template <size_t N>
concept IsPowerOfTwo = is_power_of_two(N);

/* ------------------------------------------------------------------- */
