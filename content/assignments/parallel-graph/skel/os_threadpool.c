#include "os_threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>


typedef struct thread_arg_t thread_arg_t;
struct thread_arg_t {
	// Each thread require multiple fields from the threadpool
	os_threadpool_t *tp;
};

thread_arg_t *threads_args;


/* Creates a task that thread must execute */
os_task_t *task_create(void *arg, void (*f)(void *))
{
	os_task_t *new_task = calloc(1, sizeof(os_task_t));

	if (new_task == NULL) {
		perror("Error creating task");
		exit(1);
	}
	new_task->argument = arg;
	new_task->task = f;

	return new_task;
}

/* Add a new task to threadpool task queue */
void add_task_in_queue(os_threadpool_t *tp, os_task_t *t)
{
	// Get the head of the queue
	os_task_queue_t *task_node = tp->tasks;

	// Create a new node
	os_task_queue_t *new_node = calloc(1, sizeof(os_task_queue_t));

	if (new_node == NULL) {
		perror("Error creating task node");
		exit(1);
	}

	new_node->task = t;
	new_node->next = NULL;

	// First task node added -> set the head of the queue
	if (task_node == NULL) {
		tp->tasks = new_node;
		return;
	}

	// Find the last position where to add the task node
	while (task_node->next != NULL)
		task_node = task_node->next;

	// Create the links between nodes
	task_node->next = new_node;
}

/* Get the head of task queue from threadpool */
os_task_t *get_task(os_threadpool_t *tp)
{
	if (tp == NULL)
		return NULL;

	if (tp->tasks == NULL)
		return NULL;

	os_task_queue_t *head = tp->tasks;

	tp->tasks = tp->tasks->next;
	return head->task;
}

/* === THREAD POOL === */

/* Initialize the new threadpool */
os_threadpool_t *threadpool_create(unsigned int nTasks, unsigned int nThreads)
{
	os_threadpool_t *tp = calloc(1, sizeof(os_threadpool_t));

	if (tp == NULL) {
		perror("Error creating threadpool");
		exit(1);
	}

	// Syncronization elemnts
	pthread_mutex_init(&tp->taskLock, NULL);

	tp->should_stop = 0;
	tp->num_threads = nThreads;
	tp->tasks = NULL;

	// Allocate memory for threads
	tp->threads = calloc(nThreads, sizeof(pthread_t));
	if (tp->threads == NULL) {
		perror("Error creating threads");
		exit(1);
	}

	// Allocate memory for threads arguments
	threads_args = calloc(nThreads, sizeof(thread_arg_t));
	if (threads_args == NULL) {
		perror("Error creating threads");
		exit(1);
	}

	// Create N threads
	for (int i = 0; i < nThreads; i++) {
		threads_args[i].tp = tp;
		pthread_create(&tp->threads[i], NULL, thread_loop_function, &threads_args[i]);
	}

	return tp;
}

/* Loop function for threads */
void *thread_loop_function(void *args)
{
	// Get the arguments of the current thread
	thread_arg_t *arg = (thread_arg_t *) args;
	os_threadpool_t *tp = arg->tp;
	pthread_mutex_t *mutex = &arg->tp->taskLock;

	// Loop until the threadpool is stopped
	while (tp->should_stop == 0) {
		// Get a task from the queue and execute it
		pthread_mutex_lock(mutex);
		os_task_t *task = get_task(tp);

		pthread_mutex_unlock(mutex);

		if (task != NULL) {
			void *argument = task->argument;

			task->task(argument);
		}
	}

	return NULL;
}

/* Stop the thread pool once a condition is met */
void threadpool_stop(os_threadpool_t *tp, int (*processingIsDone)(os_threadpool_t *))
{
	// Wait for all threads to finish their tasks
	while (processingIsDone(tp) == 0);

	tp->should_stop = 1;
	// Stop the threadpool
	for (int i = 0; i < tp->num_threads; i++)
		pthread_join(tp->threads[i], NULL);
}
