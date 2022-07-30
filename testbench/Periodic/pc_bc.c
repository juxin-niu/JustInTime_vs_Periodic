
#include <testbench/global_declaration.h>
#include <testbench/Periodic_scheduler.h>
#include <testbench/testbench_api.h>

__SHARED_VAR(
uint32_t _v_seed;
uint16_t _v_iter;
uint16_t _v_func;
uint16_t _v_n_0;
uint16_t _v_n_1;
uint16_t _v_n_2;
uint16_t _v_n_3;
uint16_t _v_n_4;
uint16_t _v_n_5;
uint16_t _v_n_6;
)

static __nv bool      first_run = 1;
static __nv uint16_t  status = 0;
static const uint16_t global_war_size = 11;
static __nv uint16_t  backup_buf[11] = {};


static const bool backup_needed[] = {
    false, true, true, true, true, true, true, true, true
};

void pc_bc_main()
{
    // Local variables
    uint32_t tmp_seed;
    uint32_t temp;
    uint16_t __cry;
    union { unsigned char ch[4]; long y; } U;
    unsigned char * Ptr;
    int Accu;
    unsigned i, nn;

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
    case 1: goto Func_Select;
    case 2: goto func0;
    case 3: goto func1;
    case 4: goto NTBL0;
    case 5: goto NTBL1;
    case 6: goto BW_BTBL;
    case 7: goto AR_BTBL;
    case 8: goto Bit_Shifter;
    }

    // Tasks
    // =================================================================
    __BUILDIN_TASK_BOUNDARY(0, init);

    __GET(_v_func) = 0;
    __GET(_v_n_0) = 0;
    __GET(_v_n_1) = 0;
    __GET(_v_n_2) = 0;
    __GET(_v_n_3) = 0;
    __GET(_v_n_4) = 0;
    __GET(_v_n_5) = 0;
    __GET(_v_n_6) = 0;
    __NEXT(1, Func_Select);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(1, Func_Select);

    __GET(_v_seed) = (uint32_t)BITCOUNT_SEED;
    __GET(_v_iter) = 0;
    switch(__GET(_v_func)++)
    {
    case 0:     { __NEXT(2, func0); }
    case 1:     { __NEXT(3, func1); }
    case 2:     { __NEXT(4, NTBL0); }
    case 3:     { __NEXT(5, NTBL1); }
    case 4:     { __NEXT(6, BW_BTBL); }
    case 5:     { __NEXT(7, AR_BTBL); }
    case 6:     { __NEXT(8, Bit_Shifter); }
    default:    { __FINISH; }
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(2, func0);

    tmp_seed = __GET(_v_seed);
    __GET(_v_seed) = tmp_seed + 13;
    temp = 0;

    if (tmp_seed) do
       temp++;
    while ( 0 != (tmp_seed = tmp_seed & (tmp_seed - 1)) );

    __GET(_v_n_0) += temp;
    __GET(_v_iter)++;
    if (__GET(_v_iter) < BITCOUNT_ITER) {
       __NEXT(2, func0);
    }
    else {
       __NEXT(1, Func_Select);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(3, func1);

    tmp_seed = __GET(_v_seed);
    __GET(_v_seed) = tmp_seed + 13;

    tmp_seed = ((tmp_seed & 0xAAAAAAAAL) >>  1) + (tmp_seed & 0x55555555L);
    tmp_seed = ((tmp_seed & 0xCCCCCCCCL) >>  2) + (tmp_seed & 0x33333333L);
    tmp_seed = ((tmp_seed & 0xF0F0F0F0L) >>  4) + (tmp_seed & 0x0F0F0F0FL);
    tmp_seed = ((tmp_seed & 0xFF00FF00L) >>  8) + (tmp_seed & 0x00FF00FFL);
    tmp_seed = ((tmp_seed & 0xFFFF0000L) >> 16) + (tmp_seed & 0x0000FFFFL);

    __GET(_v_n_1) += (int)tmp_seed;
    __GET(_v_iter)++;
    if (__GET(_v_iter) < BITCOUNT_ITER) {
        __NEXT(3, func1);
    }
    else {
        __NEXT(1, Func_Select);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(4, NTBL0);

    tmp_seed = __GET(_v_seed);
    __GET(_v_n_2) += Bitcount_NonRecursiveCnt(tmp_seed);
    __GET(_v_seed) = tmp_seed + 13;

    __GET(_v_iter)++;
    if(__GET(_v_iter) < BITCOUNT_ITER) {
       __NEXT(4, NTBL0);
    }
    else {
        __NEXT(1, Func_Select);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(5, NTBL1);

    __cry = __GET(_v_seed);

    __GET(_v_n_3) += bc_bits[ (int) (__cry & 0x0000000FUL)] +
            bc_bits[ (int)((__cry & 0x000000F0UL) >> 4) ] +
            bc_bits[ (int)((__cry & 0x00000F00UL) >> 8) ] +
            bc_bits[ (int)((__cry & 0x0000F000UL) >> 12)] +
            bc_bits[ (int)((__cry & 0x000F0000UL) >> 16)] +
            bc_bits[ (int)((__cry & 0x00F00000UL) >> 20)] +
            bc_bits[ (int)((__cry & 0x0F000000UL) >> 24)] +
            bc_bits[ (int)((__cry & 0xF0000000UL) >> 28)] ;

    tmp_seed = __GET(_v_seed);
    __GET(_v_seed) = tmp_seed + 13;
    __GET(_v_iter)++;
    if(__GET(_v_iter) < BITCOUNT_ITER) {
        __NEXT(5, NTBL1);
    }
    else {
        __NEXT(1, Func_Select);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(6, BW_BTBL);

    U.y = __GET(_v_seed);
    __GET(_v_n_4) += bc_bits[ U.ch[0] ] + bc_bits[ U.ch[1] ] +
            bc_bits[ U.ch[3] ] + bc_bits[ U.ch[2] ];

    tmp_seed = __GET(_v_seed);
    __GET(_v_seed) = tmp_seed + 13;
    __GET(_v_iter)++;
    if(__GET(_v_iter) < BITCOUNT_ITER) {
        __NEXT(6, BW_BTBL);
    }
    else {
        __NEXT(1, Func_Select);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(7, AR_BTBL);

    Ptr = (unsigned char *) &__GET(_v_seed);
    Accu  = bc_bits[ *Ptr++ ];
    Accu += bc_bits[ *Ptr++ ];
    Accu += bc_bits[ *Ptr++ ];
    Accu += bc_bits[ *Ptr ];
    __GET(_v_n_5) += Accu;

    tmp_seed = __GET(_v_seed);
    __GET(_v_seed) = tmp_seed + 13;
    __GET(_v_iter)++;
    if(__GET(_v_iter) < BITCOUNT_ITER) {
        __NEXT(7, AR_BTBL);
    }
    else {
        __NEXT(1, Func_Select);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(8, Bit_Shifter);

    tmp_seed = __GET(_v_seed);
    for (i = nn = 0;
           tmp_seed && (i < (sizeof(long) * BITCOUNT_CHARBIT));
           ++i, tmp_seed >>= 1)
    {
       nn += (unsigned)(tmp_seed & 1L);
    }

    __GET(_v_n_6) += nn;

    tmp_seed = __GET(_v_seed);
    __GET(_v_seed) = tmp_seed + 13;
    __GET(_v_iter)++;
    if(__GET(_v_iter) < BITCOUNT_ITER) {
        __NEXT(8, Bit_Shifter);
    }
    else {
        __NEXT(1, Func_Select);
    }

}
