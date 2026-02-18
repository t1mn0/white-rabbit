## Local Lock-Free SPMC Deque

Sharded state for each worker to avoid contention

- LIFO policy from worker's perspective (cache locality)
- FIFO policy from stealer's perspective

- Since tasks are intrusive => tasks live outside the deque
- => Deque stores only pointers to intrusive tasks (`TaskBase*`)
  - intrusiveness comes to avoid any unnecessary heap allocation
  - state of the execution unit migrate from the heap allocared std::function to the task itself

- Deque is bounded => storage for tasks based on the Ring Buffer with static capacity
  - capacity is a power of two due to the simplicity and speed of the atomic AND operation (&)
