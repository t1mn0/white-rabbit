#pragma once

#include <vvv/list.hpp>

#include "interface.hpp"

namespace wr {

struct TaskBase : public task::detail::ITask, vvv::IntrusiveListNode<TaskBase> {};

}  // namespace wr
