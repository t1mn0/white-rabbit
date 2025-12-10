#pragma once

#include <atomic>

#include "task.hpp"

namespace wr {

template<typename TaskType> class TaskPool;

using QueueItem = ITask*;

struct QueueNode {
  QueueItem data;
  std::atomic<QueueNode*> next;

  QueueNode(QueueItem item) : data(item), next(nullptr) {}
};

class UnboundedMPMCQueue {
private: // fields:
  std::atomic<QueueNode*> head;
  std::atomic<QueueNode*> tail;

public: // member-functions:
  UnboundedMPMCQueue();
  ~UnboundedMPMCQueue();

  UnboundedMPMCQueue(const UnboundedMPMCQueue&) = delete;
  UnboundedMPMCQueue& operator=(const UnboundedMPMCQueue&) = delete;

  void push(QueueItem item);
  QueueItem pop();
  int pop_batch(QueueItem* output_buffer, int max_count);
};

} // namespace wr
