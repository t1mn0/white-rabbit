## Coordinator
The `Сoordinator` module is used to orchestrate __Workers__ who are looking for a task (to steal it) from other Workers. Coordinator makes sure that the number of workers stealing at the same time (so that the workers do not endlessly steal tasks from each other) does not exceed half of the number of all workers (TODO: can we make the maximum number of stealer-workers a configurable policy?). 

It also gives a `Park` directive to a Worker which will not be able to pick up tasks, and an ability to "wake up" when task appears in system. The message about the appearance of task comes from the Scheduler (`WsExecutor`) itself when a new task arrives in the [global shared queue]("../queues/global/global_queue.hpp") or on one of the Workers. Coordinator also notifies parked Worker if the Scheduler shutting down.

### Work cycle around Coordinator step by step:

![work cycle](../../docs/media/coordinator_work_cycle.png)

`Worker` --> `Coordinator` : requests directives;

1. If the `Scheduler` terminates, the `Coordinator` generates the `Terminate` response directive;
2. Otherwise: `Coordinator` refers to the `Throttler` - _tagged semaphore_, which, based on the limits set in it (when initializing the Scheduler), issues a `Permit` for stealing or does not issue it:
    2.1 Semaphore give `Permit` => a response directive is formed to the `Worker` with permission to steal (`coord::Directive::Steal`)
    2.2 Semaphore didnt give `Permit` => we should check that no tasks appeared while we were asking semaphore:
        2.2.1 `Task` appeared => returning `coord::Directive::Retry` to `Worker`;
        2.2.2 No tasks appeared => returning `coord::Directive::Park` (`Worker` will process the directive and request parking via `host().coordinator().park_worker()`, after which the worker will fall asleep on `Throttler`'s condvar)

`Coordinator` ---> `Worker` : returning directive;

### Architecture & design

`Worker` interacts only with the `Coordinator`, `Throttler` is the internal essence of the `Coordinator` and serves to encapsulate the logic of the semaphore. 
__`Coordinator`__ = Facade for `Throttler` + two-phase parking logic + receiving signals about new task in the system to waking up parked `Worker`.

`Throttler` is a tagged semaphore that limits the number of active thieves. Basic policy: `total_workers_num / 2`. The `Permit` (tag) issued by the semaphore is represented as a (_RAII-wrapped_) _linear type_ `StealPermit` object. When a `StealPermit` object is destroyed, the internal `permit-counter` in the semaphore is automatically incremented back - the `StealPermit` is considered used during destruction or a native call to `permit.release()`.

### Directives

`Coordinator` returns a response to the `Worker` in the form of `coord::Directive`. This is a type-safe response that tells the `Worker` what to do next. Possible states of the directive (`Actions`):

1. `Retry`: an indication for the special case of two-phase parking. This case occurs when a task is not found in the first iteration, but is found immediately after it;
2. `Park`: the limit of stealers is exhausted, there is no one to steal task from;
3. `Steal`: permission to steal has been obtained. `StealPermit` token is located inside the directive;
4. `Terminate`: system (Scheduler) is shutting down. The worker must complete his run-loop;
