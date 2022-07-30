
#include <testbench/global_declaration.h>
#include <testbench/Periodic_scheduler.h>
#include <testbench/testbench_api.h>

__SHARED_VAR(
uint16_t            deq_idx;
uint16_t            enq_idx;
uint16_t            node_idx;
dijkstra_node_t     node_list[DIJKSTRA_NNODES];

dijkstra_queue_t    queue[DIJKSTRA_QSIZE];
uint16_t            src_node;
dijkstra_queue_t    nearest_node;
)

static __nv bool      first_run = 1;
static __nv uint16_t  status = 0;
static const uint16_t global_war_size = 2 * DIJKSTRA_NNODES + 3;
static __nv uint16_t  backup_buf[2 * DIJKSTRA_NNODES + 3] = {};

static const bool backup_needed[] = { false, true, true };

void pc_dijkstra_main()
{
    // Local variables
    uint16_t i, j;
    uint16_t node;
    uint16_t cost;
    uint16_t nearest_dist;
    uint16_t dist;

    // Buildin scheduler

    if (first_run == 1) { status = 0; first_run = 0;}
    else {
        if (__IS_TASK_RUNNING && backup_needed[__GET_CURTASK] == true) {
            BUILDIN_UNDO;
        }
    }

    PREPARE_FOR_BACKUP;

    switch(__GET_CURTASK) {
    case 0: goto init;
    case 1: goto Nearest_Node_Sel;
    case 2: goto Shorter_Path_Find;
    }

    // Tasks

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(0, init);
    __GET(src_node) = 0;

    __GET(queue[0].node) = __GET(src_node);
    __GET(queue[0].dist) = 0;
    __GET(queue[0].prev) = DIJKSTRA_UNDEFINED;
    __GET(deq_idx) = 0;
    __GET(enq_idx) = 1;

    for(i = 0; i < DIJKSTRA_NNODES; ++i)
    {
         __GET(node_list[i].dist) = DIJKSTRA_INFINITY;
         __GET(node_list[i].prev) = DIJKSTRA_UNDEFINED;
    }

    __GET(node_list[__GET(src_node)].dist) = 0;
    __GET(node_list[__GET(src_node)].prev) = DIJKSTRA_UNDEFINED;

    __NEXT(1, Nearest_Node_Sel);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(1, Nearest_Node_Sel);
    i = __GET(deq_idx);
    if (__GET(enq_idx) != i)
    {
       __GET(nearest_node.node) = __GET(queue[i].node);
       __GET(nearest_node.dist) = __GET(queue[i].dist);
       __GET(nearest_node.prev) = __GET(queue[i].prev);

       ++i;
       if (i < DIJKSTRA_QSIZE)  __GET(deq_idx) = i;
       else  __GET(deq_idx) = 0;

       __GET(node_idx) = 0;
       __NEXT(2, Shorter_Path_Find);
    }
    else {
        __FINISH;
    }


    // =================================================================
    __BUILDIN_TASK_BOUNDARY(2, Shorter_Path_Find);
    node = __GET(nearest_node.node);
    i = __GET(node_idx);
    cost = adj_matrix[node][i];

    if (cost != DIJKSTRA_INFINITY)
    {
       nearest_dist = __GET(nearest_node.dist);
       dist = __GET(node_list[i].dist);
       if (dist == DIJKSTRA_INFINITY || dist > (cost + nearest_dist))
       {
           __GET(node_list[i].dist) = nearest_dist + cost;
           __GET(node_list[i].prev) = node;

           j = __GET(enq_idx);
           __GET(queue[j].node) = i;
           __GET(queue[j].dist) = nearest_dist + cost;
           __GET(queue[j].prev) = node;

           j++;
           if (j < DIJKSTRA_QSIZE)  __GET(enq_idx) = j;
           else  __GET(enq_idx) = 0;

       }
    }

    if(++__GET(node_idx) < DIJKSTRA_NNODES) {
       __NEXT(2, Shorter_Path_Find);
    }
    else {
        __NEXT(1, Nearest_Node_Sel);
    }


}
