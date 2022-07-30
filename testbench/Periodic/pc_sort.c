
#include <testbench/global_declaration.h>
#include <testbench/Periodic_scheduler.h>
#include <testbench/testbench_api.h>

__SHARED_VAR(
uint16_t sorted[SORT_LENGTH];
uint16_t inner_index;
uint16_t outer_index;
)

static __nv bool      first_run = 1;
static __nv uint16_t  status = 0;
static const uint16_t global_war_size = SORT_LENGTH + 2;
static __nv uint16_t  backup_buf[SORT_LENGTH + 2] = {};

static const bool backup_needed[] = { false, true, true };

void pc_sort_main()
{
    // Local variables

    uint16_t temp;
    uint16_t val_outer;
    uint16_t val_inner;

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
    case 1: goto inner_loop;
    case 2: goto outer_loop;
    }

    // Tasks

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(0, init);
    __GET(outer_index) = 0;
    __GET(inner_index) = 1;
    for (temp = 0; temp < SORT_LENGTH; ++temp)
        __GET(sorted[temp]) = raw[temp];
    __NEXT(1, inner_loop);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(1, inner_loop);
    val_outer = __GET(sorted[__GET(outer_index)]);
    val_inner = __GET(sorted[__GET(inner_index)]);
    if (val_outer > val_inner)
    {
       temp = val_outer;
       val_outer = val_inner;
       val_inner = temp;
    }
    __GET(sorted[__GET(outer_index)]) = val_outer;
    __GET(sorted[__GET(inner_index)]) = val_inner;

    ++__GET(inner_index);
    if (__GET(inner_index) < SORT_LENGTH) {
        __NEXT(1, inner_loop);
    }
    else {
        __NEXT(2, outer_loop);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(2, outer_loop);
    ++__GET(outer_index);
    __GET(inner_index) = __GET(outer_index) + 1;
    if (__GET(outer_index) < SORT_LENGTH - 1) {
        __NEXT(1, inner_loop);
    }
    else {
        __FINISH;
    }

}
