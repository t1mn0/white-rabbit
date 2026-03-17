## WorkStealingQueue
`WorkStealingQueue` is an implementation of the _Chase-Lev Dynamic Circular Work-Stealing Deque_. Every Worker owns exactly one `WorkStealingQueue`.

`WorkStealingQueue` characteristics:
- _Lock-Free_: relies on atomic operations (CAS, Load, Store) - no `std::mutex` is used;
- _SP-MC_ (Single-Producer, Multi-Consumer): only the owner can push tasks. Multiple thieves (and the owner) can pop tasks;
- _Bounded_ & _zero-allocation_: pre-allocated circular buffer (`std::array` with power of two capacity). Pushing and popping do not allocate memory on the heap.

## Deque Behavior (LIFO/FIFO) & division of responsibility:
Owner treats it as a Stack (_LIFO_): pushes and pops from the `bottom`. Thieves treat it as a Queue (_FIFO_): steal from the `top`. This minimizing conflicts with the owner.

1. `WorkStealingQueue` (owner API): used exclusively by the Worker who owns the queue. Has the right to mutate `bottom_`.
2. `StealHandle` (thief API): lightweight object given to other Workers. Acts as a "ticket" to read from `top_` and attempt to steal.

## Ring buffer & monotonic indexing
The physical storage is a fixed-size `std::array<std::atomic<Task*>, Capacity>`. To navigate it, we use two concepts of indices: _Global_ and _Local_. To map an infinite global index to a physical array slot, we require `Capacity` to be a power of two. This allows us to replace the expensive modulo operator (%) with a lightning-fast bitwise AND (&): `Local_Index = Global_Index & (Capacity - 1)`. 

## Concurrency Model
![localq](https://github.com/t1mn0/white-rabbit-media/blob/main/work_stealing_queue.png?raw=true)
