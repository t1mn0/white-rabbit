#pragma once

#include <concepts>

namespace wr::coord {

template <typename P>
concept WakeCondition = std::invocable<P> &&
                        std::convertible_to<std::invoke_result_t<P>, bool>;

}  // namespace wr::coord
