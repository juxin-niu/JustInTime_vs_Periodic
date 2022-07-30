
#include <testbench/global_declaration.h>
#include <testbench/Periodic_scheduler.h>
#include <testbench/testbench_api.h>

__SHARED_VAR(
uint16_t       _v_letter_idx;
cem_sample_t   _v_prev_sample;
cem_index_t    _v_sample_count;
cem_index_t    _v_sibling;
cem_index_t    _v_node_count;
cem_index_t    _v_out_len;

cem_letter_t   _v_letter;
cem_sample_t   _v_sample;
cem_index_t    _v_child;
cem_index_t    _v_parent;
cem_index_t    _v_parent_next;
cem_node_t     _v_parent_node;
cem_node_t     _v_sibling_node;
cem_index_t    _v_symbol;

cem_node_t     _v_compressed_data[CEM_BLOCK_SIZE];
cem_node_t     _v_dict[CEM_DICT_SIZE];
)

static __nv bool      first_run = 1;
static __nv uint16_t  status = 0;
static const uint16_t global_war_size = 6;
static __nv uint16_t  backup_buf[6] = {};


static const bool backup_needed[] = {
    false, false, true, true, true, true, true, true
};

void pc_cem_main()
{
    // Local variables
    uint16_t i;
    uint16_t next_letter_idx;
    cem_sample_t prev_sample;
    cem_sample_t sample;
    uint16_t letter_idx;
    uint16_t letter_shift;
    cem_letter_t letter;
    cem_index_t parent;
    int16_t idx;
    cem_index_t starting_node_idx;
    cem_index_t next_sibling;
    cem_index_t child;
    cem_index_t last_sibling;

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
    case 1: goto Dict_Init;
    case 2: goto Sample;
    case 3: goto Measure_Temp;
    case 4: goto Letterize_and_compress;
    case 5: goto Find_Sibling;
    case 6: goto Add_Node;
    case 7: goto Add_Insert;
    }

    // Tasks
    // =================================================================
    __BUILDIN_TASK_BOUNDARY(0, init);

    __GET(_v_parent_next) = 0;
    __GET(_v_out_len) = 0;
    __GET(_v_letter) = 0;
    __GET(_v_prev_sample) = 0;
    __GET(_v_letter_idx) = 0;
    __GET(_v_sample_count) = 1;

    __NEXT(1, Dict_Init);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(1, Dict_Init);


    for (i = 0; i < CEM_NUM_LETTERS; ++i)
    {
       __GET(_v_dict[i].letter) = i;
       __GET(_v_dict[i].sibling) = CEM_NIL;
       __GET(_v_dict[i].child) = CEM_NIL;
    }

    __GET(_v_letter) = CEM_NUM_LETTERS + 1;
    __GET(_v_node_count) = CEM_NUM_LETTERS;

    __NEXT(2, Sample);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(2, Sample);
    next_letter_idx = __GET(_v_letter_idx) + 1;
    if (next_letter_idx == CEM_NUM_LETTERS_IN_SAMPLE)
       next_letter_idx = 0;

    if (__GET(_v_letter_idx) == 0)
    {
       __GET(_v_letter_idx) = next_letter_idx;
       __NEXT(3, Measure_Temp);
    }
    else
    {
       __GET(_v_letter_idx) = next_letter_idx;
       __NEXT(4, Letterize_and_compress);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(3, Measure_Temp);
    prev_sample = __GET(_v_prev_sample);
    sample = CEM_AcquireSample(prev_sample);
    prev_sample = sample;
    __GET(_v_prev_sample) = prev_sample;
    __GET(_v_sample) = sample;

    __NEXT(4, Letterize_and_compress);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(4, Letterize_and_compress);
    letter_idx = __GET(_v_letter_idx);
    if (letter_idx == 0)
       letter_idx = CEM_NUM_LETTERS_IN_SAMPLE;
    else
       letter_idx--;

    letter_shift = CEM_LETTER_SIZE_BITS * letter_idx;
    letter = (__GET(_v_sample) & (CEM_LETTER_MASK << letter_shift) ) >> letter_shift;
    __GET(_v_letter) = letter;

    parent = __GET(_v_parent_next);
    __GET(_v_parent_node.letter) = __GET(_v_dict[parent].letter);
    __GET(_v_parent_node.sibling) = __GET(_v_dict[parent].sibling);
    __GET(_v_parent_node.child) = __GET(_v_dict[parent].child);
    __GET(_v_sibling) = __GET(_v_dict[parent].child);
    __GET(_v_parent) = parent;
    __GET(_v_child) = __GET(_v_dict[parent].child);
    __GET(_v_sample_count)++;

    __NEXT(5, Find_Sibling);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(5, Find_Sibling);
    if (__GET(_v_sibling) != CEM_NIL) {
       idx = __GET(_v_sibling);
       if(__GET(_v_dict[idx].letter) == __GET(_v_letter)) {
           __GET(_v_parent_next) = __GET(_v_sibling);
           __NEXT(4, Letterize_and_compress);
       }
       else {
           if (__GET(_v_dict[idx].sibling) != 0) {
               __GET(_v_sibling) = __GET(_v_dict[idx].sibling);
               __NEXT(5, Find_Sibling);
           }
       }
    }

    starting_node_idx = (cem_index_t)__GET(_v_letter);
    __GET(_v_parent_next) = starting_node_idx;
    if (__GET(_v_child) == CEM_NIL) {
       __NEXT(7, Add_Insert);
    }
    else {
        __NEXT(6, Add_Node);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(6, Add_Node);
    i = __GET(_v_sibling);
    if (__GET(_v_dict[i].sibling) != CEM_NIL)
    {
       next_sibling = __GET(_v_dict[i].sibling);
       __GET(_v_sibling) = next_sibling;
       __NEXT(6, Add_Node);
    }
    else
    {
       __GET(_v_sibling_node.letter) = __GET(_v_dict[i].letter);
       __GET(_v_sibling_node.sibling) = __GET(_v_dict[i].sibling);
       __GET(_v_sibling_node.child) = __GET(_v_dict[i].child);
       __NEXT(7, Add_Insert);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(7, Add_Insert);
    if (__GET(_v_node_count) == CEM_DICT_SIZE)
       while(1){}

    child = __GET(_v_node_count);
    if (__GET(_v_parent_node.child) == CEM_NIL)
    {
       i = __GET(_v_parent);
       __GET(_v_dict[i].letter) = __GET(_v_parent_node.letter);
       __GET(_v_dict[i].sibling) = __GET(_v_parent_node.sibling);
       __GET(_v_dict[i].child) = child;
    }
    else
    {
       last_sibling = __GET(_v_sibling);
       __GET(_v_dict[last_sibling].letter) = __GET(_v_sibling_node.letter);
       __GET(_v_dict[last_sibling].sibling) = child;
       __GET(_v_dict[last_sibling].child) = __GET(_v_sibling_node.child);
    }

    __GET(_v_dict[child].letter) = __GET(_v_letter);
    __GET(_v_dict[child].sibling) = CEM_NIL;
    __GET(_v_dict[child].child) = CEM_NIL;
    __GET(_v_symbol) = __GET(_v_parent);
    __GET(_v_node_count)++;

    __GET(_v_compressed_data[__GET(_v_out_len)].letter) = __GET(_v_symbol);
    __GET(_v_out_len)++;
    if(__GET(_v_out_len) == CEM_BLOCK_SIZE) {
        __FINISH;
    }
    else{
        __NEXT(2, Sample);
    }


}
