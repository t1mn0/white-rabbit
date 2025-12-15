#pragma once

#include <cstddef>

template <size_t N>
concept PowerOfTwo = (N & (N - 1)) == 0;
