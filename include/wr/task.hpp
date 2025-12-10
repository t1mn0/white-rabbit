#pragma once

template <typename T>
class IntrusiveListNode {
  // . . .
};

namespace wr {

// Simplest atomic execution unit:
struct ITask {
public: // member-functions:
  virtual void run() = 0; // run exec unit;
  virtual void on_complete() = 0; // rescheduling / done / etc;
  //                 ^
  // This method helps scheduler to process behavior of sync/async tasks.
  // In sync tasks it is
  //
  virtual ~ITask() = default;
};

struct TaskBase : ITask, IntrusiveListNode<TaskBase> {

};

} // namespace wr
