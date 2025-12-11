#pragma once

#include "../../tasks/task.hpp"
#include <atomic>

#ifdef __cpp_lib_hardware_interference_size
inline constexpr std::size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
inline constexpr std::size_t CACHE_LINE_SIZE = 64;
#endif

namespace wr {

/* forward declaration */
template <typename TaskType>
class TaskPool;

class LockFreeMpMcQueue {

  public:  // member-functions:
    LockFreeMpMcQueue();
    ~LockFreeMpMcQueue();

    LockFreeMpMcQueue(const LockFreeMpMcQueue&) = delete;
    LockFreeMpMcQueue& operator=(const LockFreeMpMcQueue&) = delete;

    void push(ITask* item);
    ITask* pop();
    int pop_batch(ITask** output_buffer, int max_count);

    Stealer stealer();

  private:
    struct QueueNode {
        ITask* data_;
        std::atomic<QueueNode*> next_{nullptr};

        QueueNode(ITask* item);
    };

    alignas(CACHE_LINE_SIZE) std::atomic<QueueNode*> head_;
    alignas(CACHE_LINE_SIZE) std::atomic<QueueNode*> tail_;
};

}  // namespace wr
