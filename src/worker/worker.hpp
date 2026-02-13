#pragma once

#include <optional>

#include "../exec/executor.hpp"
#include "../queues/local/ws_queue.hpp"

namespace wr {

template <task::Task TaskType>
class Worker {
  public:  // member-functions:
    Worker(WsExecutor<TaskType>& host, size_t worker_index);

    void start();  // auto-join;
    void wake();   // if parked;

    // Worker is producer for its local queue;
    void push_task(TaskType* /*, SchedHint */);

    std::optional<vvv::IntrusiveList<TaskType>> yawn_tasks(size_t requested_size);
    WsExecutor<TaskType>& host() const;

  private:  // member-functions:
    void push_to_lifo_slot();
    void push_to_local_queue();
    void offload_to_global_queue();

    std::optional<TaskType*> try_pick_task();
    std::optional<TaskType*> try_pick_task_from_lifo_slot();
    std::optional<TaskType*> try_pick_task_from_local_queue();
    std::optional<TaskType*> try_pick_task_from_global_queue();

    // Using the `optional` (monadic) features we can write code like flow:
    // pick_task() = try_pick_task().or_else(/*parking*/);
    // try_pick_task() = try_pick_task_from_lifo_slot()
    //     .or_else(try_pick_task_from_local_queue())
    //     .or_else(try_pick_task_from_global_queue())
    // etc;
    TaskType* pick_task();

    void work();  // run-loop;

  private:  // static fields:
            // scheduling tick = counter, which increment every .push() call;
    static constexpr size_t GlobalQPollingInterval = 61;

  private:  // fields:
    // We introduce a ownership relationship: the executor owns the Worker objects,
    // the Worker belongs to the executor object.
    // This attitude is caused by the fact that the Worker is inextricably linked to
    // the scheduler that owns it and cannot execute its responsibility without this
    // "knowledge". On the contrary, for example, global queue may not know its
    // owner (Worker) and at the same time ideally execute its single responsibility;
    // This relationship also allows two of its participants to "look into" (=use)
    // each other's details.
    WsExecutor<TaskType>& host_;
    const size_t worker_index_;

    // tick_ field adds a bit of fairness: the worker undertakes to load a task from
    // a global queue on every constantly certain (61) iteration in order to improve
    // global progress guarantees and
    //
    // why 61?? Dmitrii Viukov (who designed and implemented the Golang scheduler) said:
    // >> "It's not even a 42!"
    // >> "not too small"
    // >> "not too big"
    // >> "prime to break any patterns"
    //
    uint64_t tick_ = 0;

    std::atomic<TaskType*> warm_slot_{nullptr}; /* LIFO */

    queues::WorkStealingQueue<TaskType*, LocalQCapacity> local_queue_;
};

};  // namespace wr
