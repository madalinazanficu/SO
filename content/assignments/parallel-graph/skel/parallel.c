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

int sum = 0;
os_graph_t *graph;
os_threadpool_t *tp;



// Function that is passed as an argument to task_create
void sum_fun (void *arg) {

    // Get the index of the node
    int index = *(int*) arg;
    os_node_t *node = graph->nodes[index];
    sum += node->nodeInfo;

    // Mark the node as visited
    graph->visited[index] = 1;


    // Submit tasks for the unvisited neighbours of the node
    for (int i = 0; i < graph->nodes[index]->cNeighbours; i++) {
        int neighbour = graph->nodes[index]->neighbours[i];
        if (graph->visited[neighbour] == 0) {
            graph->visited[neighbour] = 1;

            void *arg = &neighbour;
            os_task_t *task = task_create(sum_fun, arg);
            add_task_in_queue(tp, task);
        }
    }
}

// Function that checks if all the nodes have been processed
int processingIsDone(os_threadpool_t *tp) {
    
    for (int i = 0; i < graph->nCount; i++) {
        if (graph->visited[i] == 0) {
            return 0;
        }
    }

    return 1;
}


void traverse_graph() {

    // Step 1: Create the threadpool
    tp = threadpool_create(MAX_TASK, MAX_THREAD);

    // Step 2: Create tasks for the root of each connected component
    for (int i = 0; i < graph->nCount; i++) {
        if (graph->visited[i] == 0) {
            graph->visited[i] = 1;
            
            // Step 3: Create the task
            void *arg = &i;
            os_task_t *task = task_create(sum_fun, arg);

            // Step 4: Submit the task to the threadpool
            add_task_in_queue(tp, task);
        }
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

    printf("%d", sum);
    return 0;
}
