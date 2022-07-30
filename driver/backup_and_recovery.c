#include <driver/backup_and_recovery.h>
#include <driver/driverlib_include.h>
#include <driver/nv.h>
#include <stdint.h>

#if defined(__MSP430FR5994__)
    #define TARGET_RAM_ORIGIN       0x1C00
    #define TARGET_RAM_LENGTH       0x1000
#elif defined(__MSP430FR5969__)
    #define TARGET_RAM_ORIGIN       0x1C00
    #define TARGET_RAM_LENGTH       0x0800
#elif defined(__MSP430FR2433__)
    #define TARGET_RAM_ORIGIN       0x2000
    #define TARGET_RAM_LENGTH       0x1000
#endif

#define REGBUF_LENGTH           (15)
#define STACKBUF_LENGTH         (200)
#define SRAM_TOP                (TARGET_RAM_ORIGIN + TARGET_RAM_LENGTH + 1) // For bytes to words

static __nv uint16_t regBuf[REGBUF_LENGTH];
static __nv uint16_t stackBuf[STACKBUF_LENGTH];
static __nv uint16_t stackStartAddr;
static __nv uint16_t stackBufSizeInWord;

void backup()
{
    asm("   MOV.W   R1,  &regBuf      ");
    asm("   MOV.W   R2,  &regBuf + 2  ");
    asm("   MOV.W   R3,  &regBuf + 4  ");
    asm("   MOV.W   R4,  &regBuf + 6  ");
    asm("   MOV.W   R5,  &regBuf + 8  ");
    asm("   MOV.W   R6,  &regBuf + 10 ");
    asm("   MOV.W   R7,  &regBuf + 12 ");
    asm("   MOV.W   R8,  &regBuf + 14 ");
    asm("   MOV.W   R9,  &regBuf + 16 ");
    asm("   MOV.W   R10, &regBuf + 18 ");
    asm("   MOV.W   R11, &regBuf + 20 ");
    asm("   MOV.W   R12, &regBuf + 22 ");
    asm("   MOV.W   R13, &regBuf + 24 ");
    asm("   MOV.W   R14, &regBuf + 26 ");
    asm("   MOV.W   R15, &regBuf + 28 ");

    stackStartAddr = regBuf[0];
    stackBufSizeInWord = (SRAM_TOP - stackStartAddr) >> 1;

#if (defined(__MSP430FR5969__) || defined(__MSP430FR5994__)) && (JIT_BACKUP_MODE == BACKUP_WITH_DMA)
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) &= ~DMAEN;
    __data16_write_addr((unsigned short)(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0SA), (unsigned long)stackStartAddr);
    __data16_write_addr((unsigned short)(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0DA), (unsigned long)stackBuf);
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0SZ) = stackBufSizeInWord;
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= DMAEN;
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= DMAREQ;
#elif defined(__MSP430FR2433__) && (JIT_BACKUP_MODE == BACKUP_WITH_DMA)
#error  "MSP430FR2433 DOES NOT HAVE A DMA."
#elif JIT_BACKUP_MODE == BACKUP_WITH_CPU
    register uint16_t *srcPtr = (uint16_t *)stackStartAddr;
    register uint16_t *dstPtr = stackBuf;
    register uint16_t numberOfWords = stackBufSizeInWord;
    while (numberOfWords > 0) {
        *dstPtr++ = *srcPtr++;
        numberOfWords--;
    }
#endif

}

void recovery()
{
#if (defined(__MSP430FR5969__) || defined(__MSP430FR5994__)) && (JIT_BACKUP_MODE == BACKUP_WITH_DMA)
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) &= ~DMAEN;
    __data16_write_addr((unsigned short)(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0SA), (unsigned long)stackBuf);
    __data16_write_addr((unsigned short)(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0DA), (unsigned long)stackStartAddr);
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0SZ) = stackBufSizeInWord;
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= DMAEN;
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= DMAREQ;
#elif defined(__MSP430FR2433__) && (JIT_BACKUP_MODE == BACKUP_WITH_DMA)
#error  "MSP430FR2433 DOES NOT HAVE A DMA."
#elif JIT_BACKUP_MODE == BACKUP_WITH_CPU
    register uint16_t *srcPtr = stackBuf;
    register uint16_t *dstPtr = (uint16_t *)stackStartAddr;
    register uint16_t numberOfWords = stackBufSizeInWord;
    while (numberOfWords > 0) {
        *dstPtr++ = *srcPtr++;
        numberOfWords--;
    }
#endif


    asm("   MOV.W   &regBuf     , R1  ");
    asm("   NOP                       ");
    asm("   MOV.W   &regBuf + 2 , R2  ");
    asm("   NOP                       ");
    asm("   MOV.W   &regBuf + 4 , R3  ");
    asm("   MOV.W   &regBuf + 6 , R4  ");
    asm("   MOV.W   &regBuf + 8 , R5  ");
    asm("   MOV.W   &regBuf + 10, R6  ");
    asm("   MOV.W   &regBuf + 12, R7  ");
    asm("   MOV.W   &regBuf + 14, R8  ");
    asm("   MOV.W   &regBuf + 16, R9  ");
    asm("   MOV.W   &regBuf + 18, R10 ");
    asm("   MOV.W   &regBuf + 20, R11 ");
    asm("   MOV.W   &regBuf + 22, R12 ");
    asm("   MOV.W   &regBuf + 24, R13 ");
    asm("   MOV.W   &regBuf + 26, R14 ");
    asm("   MOV.W   &regBuf + 28, R15 ");
}
