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


void visit_node(int index);
void sum_fun (void *arg);

void visit_node(int node) {

    // Step 1: Check if the node is visited and mark it
    pthread_mutex_lock(&tp->taskLock);
    if (graph->visited[node] == 0) {
        graph->visited[node] = 1;

        // Step 2: Create the task
        os_task_t *task = task_create(&ind[node], sum_fun);

        // Step 3: Submit the task to the threadpool
        add_task_in_queue(tp, task);
    } 
    pthread_mutex_unlock(&tp->taskLock);
}


// Function that is passed as an argument to task_create
void sum_fun(void *arg) {

    // Get the index of the node
    int index = *(int*) arg;
    os_node_t *node = graph->nodes[index];

    pthread_mutex_lock(&sum_mutex);
    sum += node->nodeInfo;
    pthread_mutex_unlock(&sum_mutex);

    // Submit tasks for the unvisited neighbours of the node
    for (int i = 0; i < node->cNeighbours; i++) {
        int neighbour = node->neighbours[i];
        visit_node(neighbour);
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

void traverse_graph() {

    // Step 1: Create the threadpool
    tp = threadpool_create(MAX_TASK, MAX_THREAD);

    // Step 2: Create tasks for the root of each connected component
    for (int i = 0; i < graph->nCount; i++) {
        visit_node(i);
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
