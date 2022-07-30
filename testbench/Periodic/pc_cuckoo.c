
#include <testbench/global_declaration.h>
#include <testbench/Periodic_scheduler.h>
#include <testbench/testbench_api.h>

__SHARED_VAR(
cuckoo_value_t         _v_key;
cuckoo_value_t         _v_insert_count;
cuckoo_value_t         _v_inserted_count;
cuckoo_value_t         _v_lookup_count;
cuckoo_value_t         _v_member_count;
cuckoo_fingerprint_t   _v_filter[CUCKOO_NUM_BUCKETS];

cuckoo_value_t         _v_index1;
cuckoo_value_t         _v_index2;
cuckoo_fingerprint_t   _v_fingerprint;
cuckoo_value_t         _v_relocation_count;
uint16_t               _v_index;
TaskName               _v_next_task;
uint16_t               _v_success;
uint16_t               _v_member;
)

static __nv bool      first_run = 1;
static __nv uint16_t  status = 0;
static const uint16_t global_war_size = CUCKOO_NUM_BUCKETS + 9;
static __nv uint16_t  backup_buf[CUCKOO_NUM_BUCKETS + 9] = {};


static const bool backup_needed[] = {
    false, true, false, true, true, true, true
};

void pc_cuckoo_main()
{
    // Local variables

    uint16_t i;
    cuckoo_index_t fp_hash;
    uint16_t __cry_idx;
    uint16_t __cry_idx2;
    cuckoo_fingerprint_t fp_victim;
    cuckoo_index_t index_victim;
    cuckoo_index_t fp_hash_victim;
    cuckoo_index_t index2_victim;
    uint16_t __cry;

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
    case 1: goto KeyGenerate;
    case 2: goto shared_calc_index;
    case 3: goto Add;
    case 4: goto Relocate;
    case 5: goto Insert_Done;
    case 6: goto Lookup_Search;
    }

    // Tasks
    // =================================================================
    __BUILDIN_TASK_BOUNDARY(0, init);
    for (i = 0; i < CUCKOO_NUM_BUCKETS; ++i)
        __GET(_v_filter[i]) = 0;

    __GET(_v_insert_count) = 0;
    __GET(_v_lookup_count) = 0;
    __GET(_v_inserted_count) = 0;
    __GET(_v_member_count) = 0;
    __GET(_v_key) = cuckoo_init_key;
    __GET(_v_next_task) = CUCKOO_Insert;

    __NEXT(1, KeyGenerate);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(1, KeyGenerate);
    __GET(_v_key) = (__GET(_v_key) + 1) * 17;
    if (__GET(_v_next_task) == CUCKOO_Insert) {
        __GET(_v_next_task) = CUCKOO_Add;
        __NEXT(2, shared_calc_index);
    }
    else if (__GET(_v_next_task) == CUCKOO_Lookup) {
        __GET(_v_next_task) = CUCKOO_Lookup_Search;
        __NEXT(2, shared_calc_index);
    }
    __NEXT(2, shared_calc_index);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(2, shared_calc_index);
    __GET(_v_fingerprint) = CUCKOO_Hash2Fingerprint(__GET(_v_key));

    __GET(_v_index1) = CUCKOO_Hash2Index(__GET(_v_key));

    fp_hash = CUCKOO_Hash2Index(__GET(_v_fingerprint));
    __GET(_v_index2) = __GET(_v_index1) ^ fp_hash;

    if (__GET(_v_next_task) == CUCKOO_Add) {
        __NEXT(3, Add);
    }
    else if (__GET(_v_next_task) == CUCKOO_Lookup_Search) {
        __NEXT(6, Lookup_Search);
    }
    __GET(_v_next_task) = CUCKOO_Add;
    __NEXT(2, shared_calc_index);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(3, Add);
    __cry_idx = __GET(_v_index1);
    __cry_idx2 = __GET(_v_index2);

    if (!__GET(_v_filter[__cry_idx]))
    {
        __GET(_v_success) = 1;
        __GET(_v_filter[__cry_idx]) = __GET(_v_fingerprint);
        __NEXT(5, Insert_Done);
    }
    else
    {
        if (!__GET(_v_filter[__cry_idx2]))
        {
            __GET(_v_success) = 1;
            __GET(_v_filter[__cry_idx2]) = __GET(_v_fingerprint);
            __NEXT(5, Insert_Done);
        }
        else
        {
            if (__GET(_v_key) % 2)   //! WARNNING: replace rand() with __GET(_v_key).
            {
                index_victim = __cry_idx;
                fp_victim = __GET(_v_filter[__cry_idx]);
            }
            else
            {
                index_victim = __cry_idx2;
                fp_victim = __GET(_v_filter[__cry_idx2]);
            }
            __GET(_v_filter[index_victim]) = __GET(_v_fingerprint);
            __GET(_v_index1) = index_victim;
            __GET(_v_fingerprint) = fp_victim;
            __GET(_v_relocation_count) = 0;
        }
    }
    __NEXT(4, Relocate);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(4, Relocate);
    fp_victim = __GET(_v_fingerprint);
    fp_hash_victim = CUCKOO_Hash2Index(fp_victim);
    index2_victim = __GET(_v_index1) ^ fp_hash_victim;

    if (!__GET(_v_filter[index2_victim]))
    {
       __GET(_v_success) = 1;
       __GET(_v_filter[index2_victim]) = fp_victim;
       __NEXT(5, Insert_Done);
    }
    else
    {
       if (__GET(_v_relocation_count) >= CUCKOO_MAX_RELOCATIONS)
       {
           __GET(_v_success) = 0;
           __NEXT(5, Insert_Done);
       }

       __GET(_v_relocation_count)++;
       __GET(_v_index1) = index2_victim;
       __GET(_v_fingerprint) = __GET(_v_filter[index2_victim]);
       __GET(_v_filter[index2_victim]) = fp_victim;
       __NEXT(4, Relocate);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(5, Insert_Done);
    __GET(_v_insert_count)++;
    __cry = __GET(_v_inserted_count);
    __cry += __GET(_v_success);
    __GET(_v_inserted_count) = __cry;

    if (__GET(_v_insert_count) < CUCKOO_NUM_INSERTS)
    {
       __GET(_v_next_task) = CUCKOO_Insert;
       __NEXT(1, KeyGenerate);
    }
    else
    {
       __GET(_v_next_task) = CUCKOO_Lookup;
       __GET(_v_key) = cuckoo_init_key;
       __NEXT(1, KeyGenerate);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(6, Lookup_Search);
    if (__GET(_v_filter[__GET(_v_index1)]) == __GET(_v_fingerprint))
           __GET(_v_member) = 1;
    else if (__GET(_v_filter[__GET(_v_index2)]) == __GET(_v_fingerprint))
       __GET(_v_member) = 1;
    else
       __GET(_v_member) = 0;

    __GET(_v_lookup_count)++;
    __cry = __GET(_v_member_count);
    __cry += __GET(_v_member);
    __GET(_v_member_count) = __cry;

    if (__GET(_v_lookup_count) < CUCKOO_NUM_LOOKUPS)
    {
       __GET(_v_next_task) = CUCKOO_Lookup;
       __NEXT(1, KeyGenerate);
    }
    else
    {
        __FINISH;
    }

}
