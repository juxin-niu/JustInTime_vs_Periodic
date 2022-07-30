#include <driver/mem_write.h>
#include <driver/driverlib_include.h>

void dma_prepare(unsigned long from, unsigned long to, uint16_t number_of_words)
{
#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) &= ~DMAEN;
    __data16_write_addr((unsigned short)(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0SA), from);
    __data16_write_addr((unsigned short)(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0DA), to);
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0SZ) = number_of_words;
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= DMAEN;
#endif
    return;
}

void cpu_write(uint16_t* from, uint16_t* to, uint16_t number_of_words)
{
    while (number_of_words > 0) {
        *to++ = *from++;
        number_of_words--;
    }
}
