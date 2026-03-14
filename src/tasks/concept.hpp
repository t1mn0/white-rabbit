#pragma once

#include <concepts>

#include <ntrusive/intrusive.hpp>

namespace wr::task {

template <typename T>
concept Task = requires(T task) {
    { task.run() } noexcept -> std::same_as<void>; } && std::derived_from<T, IntrusiveListNode>;

}  // namespace wr::task
