#include <testbench/global_declaration.h>
#include <testbench/testbench_api.h>

__nv uint16_t outer_index;
__nv uint16_t inner_index;
__nv uint16_t sorted[SORT_LENGTH];

void jit_sort_main()
{
    register uint16_t val_outer;
    register uint16_t val_inner;
    register uint16_t temp;

    outer_index = 0;
    inner_index = 1;
    for (temp = 0; temp < SORT_LENGTH; ++temp)
        sorted[temp] = raw[temp];

    task_inner_loop:
    val_outer = sorted[outer_index];
    val_inner = sorted[inner_index];
    if (val_outer > val_inner)
    {
        temp = val_outer;
        val_outer = val_inner;
        val_inner = temp;
    }
    sorted[outer_index] = val_outer;
    sorted[inner_index] = val_inner;

    ++inner_index;
    if (inner_index < SORT_LENGTH)
        goto task_inner_loop;

    ++outer_index;
    inner_index = outer_index + 1;
    if (outer_index < SORT_LENGTH - 1)
        goto task_inner_loop;

    return;
}
