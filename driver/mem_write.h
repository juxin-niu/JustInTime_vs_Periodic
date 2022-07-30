/*
 * mem_write.h
 *
 *  Created on: 2022Äê7ÔÂ29ÈÕ
 *      Author: juxin
 */

#ifndef DRIVER_MEM_WRITE_H_
#define DRIVER_MEM_WRITE_H_

#include <stdint.h>

void dma_prepare(unsigned long from, unsigned long to, uint16_t number_of_words);

#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
#define DMA_start HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= DMAREQ
#elif defined(__MSP430FR2433__)
#define DMA_start ((void)0)
#endif

void cpu_write(uint16_t* from, uint16_t* to, uint16_t number_of_words);

#endif /* DRIVER_MEM_WRITE_H_ */
