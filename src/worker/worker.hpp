#pragma once

#include <optional>

#include "../queues/local/ws_queue.hpp"
#include "../sched/inteface.hpp"

namespace wr {

template <task::Task TaskT>
class Worker {
  private:  // static fields:
    static constexpr size_t LocalQCapacity = 16;
    static constexpr size_t GlobalQPollingInterval = 61;  // dimension: scheduling tick

    // scheduling tick = counter, which increment every .push() call;

  private:  // fields:
    // We introduce a ownership relationship: the IScheduler owns the Worker objects
    // (the IScheduler is the host), the Worker belongs to the IScheduler object.
    // This attitude is caused by the fact that the Worker is inextricably linked to
    // the scheduler that owns it and cannot execute its responsibility without this
    // "knowledge". On the contrary, for example, GlobalTaskQueue may not know its
    // owner (Worker) and at the same time ideally execute its single responsibility;
    // This relationship also allows two of its participants to "look into" (=use)
    // each other's details. Mnemonically, this semantics is reflected in the usage
    // of ::detail namespace:
    sched::detail::IScheduler<TaskT>& host_;
    const size_t worker_index_;

    // tick_ field adds a bit of fairness: the worker undertakes to take the load off
    // the GlobalTaskQueue, even if he was given an explicit hint (SchedHint)
    // to do something else;
    uint64_t tick_ = 0;

    TaskT* warm_slot_ = nullptr; /* LIFO */

    WorkStealingQueue<TaskT*, LocalQCapacity> local_queue_;

  public:  // member-functions:
    Worker(sched::detail::IScheduler<TaskT>& host, size_t worker_index);

    void start();  // auto-join;
    void wake();   // if parked;

    // Worker is producer for its local queue;
    void push_task(TaskT* /*, SchedHint */);

    std::optional<vvv::IntrusiveList<TaskT>> yawn_tasks(size_t requested_size);
    sched::detail::IScheduler<TaskT>& host() const;

  private:  // member-functions:
    void push_to_lifo_slot();
    void push_to_local_queue();
    void offload_to_global_queue();

    std::optional<TaskT*> try_pick_task();
    std::optional<TaskT*> try_pick_task_from_lifo_slot();
    std::optional<TaskT*> try_pick_task_from_local_queue();
    std::optional<TaskT*> try_pick_task_from_global_queue();

    // Using the `optional` (monadic) features we can write code like flow:
    // pick_task() = try_pick_task().or_else(/*parking*/);
    // try_pick_task() = try_pick_task_from_lifo_slot()
    //     .or_else(try_pick_task_from_local_queue())
    //     .or_else(try_pick_task_from_global_queue())
    // etc;
    TaskT* pick_task();

    void work();  // run-loop;
};

};  // namespace wr
