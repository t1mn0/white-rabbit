#pragma once

#include <vector>
#include <thread>
#include <memory>
#include <random>

#include "task_container.hpp"
#include "worker.hpp"
#include "task.hpp"
#include "unbounded_mpmc_queue.hpp"

class Worker;

namespace wr {

class Scheduler {
private:
  UnboundedMPMCQueue/*Blocking?*/ global_queue; // second fallback for workers;
  // (first is stealings process)

  // MemPool pool; // for non-dynamic task allocation

  uint32_t worker_count;
  std::vector<std::unique_ptr<Worker>> workers;
  std::vector<std::thread> threads;

  std::random_device rd_;
  std::mt19937 rng_;

public: // member-functions:
  static const uint32_t DEFAULT_WORKERS;
  static constexpr size_t POOL_CAPACITY = 8192;

  Scheduler(uint32_t worker_count /* = DEFAULT_WORKERS*/);
  Scheduler(const Scheduler&) = delete; // non copyable;
  Scheduler& operator=(const Scheduler&) = delete; // non copyassignable;
  Scheduler(Scheduler&&) = delete; // non movable;
  Scheduler& operator=(Scheduler&&) = delete; // // non moveassignable
  ~Scheduler();

  void submit_task(ITask*);
  void submit_task_container(ITaskContainer&&);
};

} // namespace wr
