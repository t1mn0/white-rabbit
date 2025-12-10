#pragma once

#include <variant>

#include "task.hpp"

namespace wr {

// Set of atomic exec units - tasks (dependency graph / dependency list / etc):
struct ITaskContainer {
  public:  // member-functions:
           // Should be movable to move this objects in Scheduler.TaskContainerStorage;
           //                                                              ^
           // This is storage for containers of tasks, which are not ready to execute them, but we want
           // free user from responsibility store them and consider their life-cycle;
           // this method describing how to start process task container
           // or, if explicitly running this process not needed, point what (always waker?) scheduler should wait;
           // (for example, in task-graph we should run root vertex, rest vertices will be triggered as a chain reaction)
           // std::variant<ITask*, WaitFor(waker)> start_from();

    // How to delete full completed task container from Scheduler?
    // 1. Scheduler can control state of task-containers. Hence, it need method `bool is_done()` in TaskContainer
    // for checking is this TaskContainer should be deleted from Scheduler.TaskContainerStorage;
    // 2. TaskContainer can notify Scheduler => we need:
    //    a) ref to scheduler in task_container
    //    b) method to notify scheduler
    //    c) in this method we must explicitly specify some kind of token by which Scheduler can recognize
    //       this container in his Scheduler.TaskContainerStorage;
    //    example: void .notify_completed(Scheduler& scheduler, Token){
    //                scheduler.container_is_done(Token);
    //             }
    //    where:   void .container_is_done(Token) { (is Scheduler method)
    //                container_storage.pop(Token);
    //             }
    //    where: container_storage is field in Scheduler;
};

}  // namespace wr
