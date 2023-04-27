#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <time.h>

#include "os_graph.h"
#include "os_threadpool.h"
#include "os_list.h"

#define MAX_TASK 1000
#define MAX_THREAD 4
#define MAX_NODES 1000

int sum = 0;
os_graph_t *graph;
os_threadpool_t *tp;
int ind[MAX_NODES];
pthread_mutex_t sum_mutex;


// Function that is passed as an argument to task_create
void sum_fun (void *arg) {

    // Get the index of the node
    int index = *(int*) arg;
    os_node_t *node = graph->nodes[index];

    pthread_mutex_lock(&sum_mutex);
    sum += node->nodeInfo;
    pthread_mutex_unlock(&sum_mutex);



    // Submit tasks for the unvisited neighbours of the node
    for (int i = 0; i < node->cNeighbours; i++) {
        int neighbour = node->neighbours[i];

        pthread_mutex_lock(&tp->taskLock);
        if (graph->visited[neighbour] == 0) {

            graph->visited[neighbour] = 1;

            os_task_t *task = task_create(&ind[neighbour], sum_fun);

            add_task_in_queue(tp, task);
        }
        pthread_mutex_unlock(&tp->taskLock);
    }
}

// Function that checks if all the nodes have been processed
int processingIsDone(os_threadpool_t *tp) {
    
    for (int i = 0; i < graph->nCount; i++) {
        if (graph->visited[i] == 0) {
            return 0;
        }
    }

    if (tp->tasks != NULL) {
        return 0;
    }
    return 1;
}


void visit_node(int index) {

    // One thread at a time should check if the node has been visited
    pthread_mutex_lock(&tp->taskLock);
    if (graph->visited[index] == 1) {
        pthread_mutex_unlock(&tp->taskLock);
        return;
    }
    pthread_mutex_unlock(&tp->taskLock);

    // Mark the node as visited
    pthread_mutex_lock(&tp->taskLock);
    graph->visited[index] = 1;
    pthread_mutex_unlock(&tp->taskLock);

    // Create a task for the node
    os_task_t *task = task_create(&ind[index], sum_fun);

    // Submit the task to the threadpool (adding a task is already synchronized)
    add_task_in_queue(tp, task);
}

void traverse_graph() {

    // Step 1: Create the threadpool
    tp = threadpool_create(MAX_TASK, MAX_THREAD);

    // Step 2: Create tasks for the root of each connected component
    for (int i = 0; i < graph->nCount; i++) {
        pthread_mutex_lock(&tp->taskLock);
        if (graph->visited[i] == 0) {
            graph->visited[i] = 1;

            // Step 3: Create the task
            os_task_t *task = task_create(&ind[i], sum_fun);

            // Step 4: Submit the task to the threadpool
            add_task_in_queue(tp, task);
        }
        pthread_mutex_unlock(&tp->taskLock);
    }

    // Step 5: Shutdown the threadpool using threadpool_stop
    threadpool_stop(tp, processingIsDone);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: ./main input_file\n");
        exit(1);
    }

    FILE *input_file = fopen(argv[1], "r");

    if (input_file == NULL) {
        printf("[Error] Can't open file\n");
        return -1;
    }

    graph = create_graph_from_file(input_file);
    if (graph == NULL) {
        printf("[Error] Can't read the graph from file\n");
        return -1;
    }

    // TODO: create thread pool and traverse the graf
    for (int i = 0; i < graph->nCount; i++) {
        ind[i] = i;
    }

    pthread_mutex_init(&sum_mutex, NULL);
    traverse_graph();

    printf("%d", sum);
    return 0;
}
