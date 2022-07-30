
#ifndef TESTBENCH_PERIODIC_SCHEDULER_H_
#define TESTBENCH_PERIODIC_SCHEDULER_H_

#include <driver/nv.h>
#include <driver/mem_write.h>
#include <driver/driverlib_include.h>

#include <global_config.h>
#include <stdint.h>

#if (defined(__MSP430FR5969__) || defined(__MSP430FR5994__)) && (PC_BACKUP_MODE == BACKUP_WITH_DMA)
#define PREPARE_FOR_BACKUP  dma_prepare((unsigned long)&__persistent_vars, (unsigned long)backup_buf, global_war_size)
#else
#define PREPARE_FOR_BACKUP  ((void)0)
#endif


#if PC_BACKUP_MODE == BACKUP_WITH_DMA
#define BUILDIN_UNDO \
        dma_prepare((unsigned long)backup_buf, (unsigned long)&__persistent_vars, global_war_size); \
        DMA_start
#elif PC_BACKUP_MODE == BACKUP_WITH_CPU
#define BUILDIN_UNDO        cpu_write(backup_buf, (uint16_t*)&__persistent_vars, global_war_size)
#endif



#if PC_BACKUP_MODE == BACKUP_WITH_DMA
#define BUILDIN_BACKUP      DMA_start
#elif PC_BACKUP_MODE == BACKUP_WITH_CPU
#define BUILDIN_BACKUP      cpu_write((uint16_t*)&__persistent_vars, backup_buf, global_war_size)
#endif


#define __GET_CURTASK           (status & 0x0FFF)
#define __IS_TASK_RUNNING       ((status & 0xF000) != 0)

#define __NEXT(id, name)        status = id; goto name
#define __FINISH                first_run = 1; return

#define __SHARED_VAR(...) \
        typedef struct { __VA_ARGS__ } FRAM_data_t  __attribute__ ((aligned (2))); \
        static __nv FRAM_data_t __persistent_vars; \

#define __GET(item) __persistent_vars.item

#define __BUILDIN_TASK_BOUNDARY(id, name) \
    name: \
    if (backup_needed[__GET_CURTASK] == true && !__IS_TASK_RUNNING) { \
        BUILDIN_BACKUP; \
    } \
    status |= 0xF000




#endif /* TESTBENCH_PERIODIC_SCHEDULER_H_ */
