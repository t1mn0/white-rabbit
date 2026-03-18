## GlobalQueue
`GlobalQueue` serves as a fallback source of task for Workers when their _local queues_ are empty. It is considered the slowest path for Worker, until it starts stealing tasks. Workers always prefer to use local queues without locks (fastest path), accessing the `GlobalQueue` only periodically (for fairness, for example, every [61](../../worker/worker.hpp) clock cycles) or when their local fires are completely exhausted.

![globalq](https://github.com/t1mn0/white-rabbit-media/blob/main/global_queue.png?raw=true)

`GlobalQueue` characteristics:

- _Blocking_: uses `std::mutex` for synchronization;
- _MP-MC_ (Multi-Producer, Multi-Consumer): safe to be accessed by any number of Workers by blocking;
- _Unbounded_: does not have a fixed capacity limit. It grows dynamically as tasks are pushed;
- _Allocation-free_ (based on intrusive queue): Uses `ntrusive::IntrusiveList`, meaning push and pop operations do not trigger any heap memory allocations (`new`/`delete`);

> `GlobalQueue` does not contain `std::condition_variable`. __It acts solely as a thread-safe data container.__ Waking up sleeping workers is the direct responsibility of the `Coordinator`.

### Intrusiveness
To manipulate `Task` objects in the context of queues and other possible intrusive data structures, simply need to rearrange the embedded pointers to the `Node`, which frees you from memory allocations. To capture this part of the design in the code Tasks implement the `Task` [concept](../../tasks/concept.hpp), meaning its derived from `ntrusive::IntrusiveListNode`.
![intrusiveness](https://github.com/t1mn0/white-rabbit-media/blob/main/intrusiveness.png?raw=true)
