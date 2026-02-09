#pragma once

#include <concepts>
#include <vvv/list.hpp>

namespace wr::task {

template <typename T>
concept Task = requires(T task) {
    { task.run() } noexcept -> std::same_as<void>; } && std::derived_from<T, vvv::IntrusiveListNode<T>>;

}  // namespace wr::task
