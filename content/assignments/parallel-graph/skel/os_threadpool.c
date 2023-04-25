#include "os_threadpool.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

typedef struct thread_arg_t thread_arg_t;
struct thread_arg_t {
    pthread_mutex_t *mutex;
    os_threadpool_t *tp;
};

// Syncronization elements
pthread_mutex_t q_mutex = PTHREAD_MUTEX_INITIALIZER;


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
    new_node->task = t;
    new_node->next = NULL;


    // First task node added
    pthread_mutex_lock(&q_mutex);
    if (task_node == NULL) {

        // Set the head of the queue
        tp->tasks = new_node;
        pthread_mutex_unlock(&q_mutex);
        return;
    }
    pthread_mutex_unlock(&q_mutex);


    // Find the last position where to add the task node
    pthread_mutex_lock(&q_mutex);
    while (task_node->next != NULL) {
        task_node = task_node->next;
    }
    pthread_mutex_unlock(&q_mutex);


    // Create the links between nodes
    task_node->next = new_node;
}

/* Get the head of task queue from threadpool */
os_task_t *get_task(os_threadpool_t *tp)
{
    
    pthread_mutex_lock(&q_mutex);
    if (tp->tasks == NULL) {
        return NULL;
    }

    os_task_queue_t *head = tp->tasks;
    tp->tasks = tp->tasks->next;
    return head;

    pthread_mutex_unlock(&q_mutex);
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

    tp->should_stop = 0;
    tp->num_threads = nThreads;
    tp->tasks = NULL;

    // Allocate memory for threads
    tp->threads = calloc(nThreads, sizeof(pthread_t));
    thread_arg_t threads_args[nThreads];

    // Syncronization elemnts
    pthread_mutex_t mutex;
    p_thread_mutex_init(&mutex, NULL);

    // Create N threads
    for (int i = 0; i < nThreads; i++) {
        thread_arg_t arg;
        arg.mutex = &mutex;
        arg.tp = tp;
        threads_args[i] = arg;

        pthread_create(&tp->threads[i], NULL, thread_loop_function, &threads_args[i]);
    }
    
    return tp;
}

/* Loop function for threads */
void *thread_loop_function(void *args)
{
    // Get the arguments of the current thread
    thread_arg_t *arg = (thread_arg_t *) args;
    pthread_mutex_t *mutex = arg->mutex;
    os_threadpool_t *tp = arg->tp;

    // Get a task from the queue and execute it
    os_task_t *node_task = get_task(tp);
    int index = node_task->argument;
    node_task->task(index);

    return NULL;
}

/* Stop the thread pool once a condition is met */
void threadpool_stop(os_threadpool_t *tp, int (*processingIsDone)(os_threadpool_t *))
{
    // Wait for all threads to finish their tasks
    int isDone = processingIsDone(tp);

    // Stop the threadpool
    if (isDone == 1) {
        tp->should_stop = 1;

        // Wait for all threads to finish
        for (int i = 0; i < tp->num_threads; i++) {
            pthread_join(tp->threads[i], NULL);
        }
    }

    // TODO : Find a way to reacall processingIsDone() function
    //       after a certain amount of time
}
