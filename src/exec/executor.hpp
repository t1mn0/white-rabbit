#pragma once

#include "../tasks/concept.hpp"
#include "../worker/worker.hpp"
#include <atomic>
#include <deque>

namespace wr {

template <task::Task TaskType>
class WsExecutor {

  private:
    std::deque<Worker<TaskType>> workers_;
    std::atomic<int> active_stealers_{0};
    size_t max_stealers_;
};


}  // namespace wr
