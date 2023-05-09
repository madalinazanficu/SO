### Zanficu Madalina - Homework 3 - Parallel Graph

# Description
This assignment aims to offer a generic implementation for a threadpool in C: 
**os_threadpool.c**
In order to test the threadpool core functionalities, **parallel.c** contains
an approach of traversing a graph in parallel and compute the sum of the nodes.

# Design choices and implementation
* Each task receives an argument, which is the index of a node to be processed
* Each task receives a pointer to a function (sum_fun in this case) and the
thread that resolves that task is in charge of calling this function.
* The threadpool is implemented using a queue of tasks, I have used
the implementation provided by os_task_queue_t *tasks from os_threadpool_t.

In order to keep it generic and respect the skeleton provided these
are some helper functions implemented:

1. sum_fun (parallel.c) which is responsible for: 
- adding the current node value to the overall sum
- creating new task for neighbours nodes

2. processingIsDone (parallel.c) which constantly checks if the threadpool 
should be stopped. The threadpool could be stopped when all the nodes are 
processed and there are no tasks left in the queue.

Processed = visited and added to the queue of the threadpool.

3. traverse_graph is in charge of submitting the first task for each 
connected components. 
This was a corner case, because the graph may not be conex.

4. visit_node was created in order to reuse code.
I used this in both traverse_graph and sum_fun. 

The role of this task is to submit a task into threadpool's queue:

- This function is using a mutex in order to assure that multiple threads are
not processing the same node at a time (for visited vector).
- Then in case the node is not visited, a new task is created 
(thread-safe operation).
- In order to sumbit and process the node, I am using a lock (tp->taskLock)
which is protecting the threadpool queue.


# Syncronizations elements
1. I have used a mutex (sum_mutex) for summing up the values in sum_fun,
since multiple threads can access sum resource at once.

2. Both adding and getting elements to the threadpool queue is restricted
by tp->taskLock. 

3. I have used the same mutex for visited vector, in order to assure that
multiple threads are not processing the same node at a time.
