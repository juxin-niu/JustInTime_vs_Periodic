
#include <driver/driverlib_include.h>
#include <driver/target.h>
#include <stdint.h>

void power_on_init()
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watch-dog timer
    PM5CTL0 &= ~LOCKLPM5;       // Disable the GPIO power-on default high-impedance mode

#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__) || defined(__MSP430FR2433__)
    P1DIR = 0xFF; P1OUT = 0x00;
    P2DIR = 0xFF; P2OUT = 0x00;
    P3DIR = 0xFF; P3OUT = 0x00;
#endif
#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
    P4DIR = 0xFF; P4OUT = 0x00;
    PJDIR = 0xFFFF; PJOUT = 0x0000;
#endif
#if defined(__MSP430FR5994__)
    P5DIR = 0xFF; P5OUT = 0x00;
    P6DIR = 0xFF; P6OUT = 0x00;
    P7DIR = 0xFF; P7OUT = 0x00;
    P8DIR = 0xFF; P8OUT = 0x00;
#endif

}


#if defined(__MSP430FR5969__) || defined(__MSP430FR5994__)
void clock_sys_init()
{

    CS_setDCOFreq(CS_DCORSEL_0, CS_DCOFSEL_0);                              // DCO in 1MHz
    CS_initClockSignal(CS_MCLK,   CS_DCOCLK_SELECT,   CS_CLOCK_DIVIDER_1);  // MCLK = DCO = 1MHz
    CS_initClockSignal(CS_SMCLK,  CS_DCOCLK_SELECT,   CS_CLOCK_DIVIDER_1);  // SMCLK = DCO = 1MHz

    CS_initClockSignal(CS_ACLK,   CS_VLOCLK_SELECT,   CS_CLOCK_DIVIDER_1);
    CS_turnOffLFXT();
}
#elif defined(__MSP430FR2433__)

#define MCLK_FREQ_MHZ 1                     // MCLK = 1MHz

void Software_Trim();

void clock_sys_init()
{
    __bis_SR_register(SCG0);                 // disable FLL
    CSCTL3 |= SELREF__REFOCLK;               // Set REFO as FLL reference source

    CSCTL1 = DCOFTRIMEN | DCOFTRIM0 | DCOFTRIM1 | DCORSEL_0;// DCOFTRIM=3, DCO Range = 1MHz
    CSCTL2 = FLLD_0 + 30;                   // DCODIV = 1MHz

    __delay_cycles(3);
    __bic_SR_register(SCG0);                 // enable FLL
    Software_Trim();                         // Software Trim to get the best DCOFTRIM value

    CSCTL4 = SELMS__DCOCLKDIV | SELA__REFOCLK; // set default REFO(~32768Hz) as ACLK source, ACLK = 32768Hz
                                             // default DCODIV as MCLK and SMCLK source

    CSCTL5 |= DIVM_0 | DIVS_0;               // SMCLK = MCLK = DCOCLK
    CS_turnOffXT1();

}
#endif



#if defined(__MSP430FR5969__) || defined(__MSP430FR5994__)
void dma_init()
{
    DMA_initParam DMAInitParam = {0};
    DMAInitParam.channelSelect = DMA_CHANNEL_0;
    DMAInitParam.transferModeSelect = DMA_TRANSFER_REPEATED_BLOCK;
    DMAInitParam.transferUnitSelect = DMA_SIZE_SRCWORD_DSTWORD;
    DMAInitParam.triggerSourceSelect = DMA_TRIGGERSOURCE_0;
    DMAInitParam.triggerTypeSelect = DMA_TRIGGER_RISINGEDGE;
    DMA_init(&DMAInitParam);
    /* Source address is incremented */
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) &= ~(DMASRCINCR_3);
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= DMA_DIRECTION_INCREMENT;
    /* Destination address is incremented */
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) &= ~(DMADSTINCR_3);
    HWREG16(DMA_BASE + DMA_CHANNEL_0 + OFS_DMA0CTL) |= (DMA_DIRECTION_INCREMENT << 2);
}
#elif defined(__MSP430FR2433__)
void dma_init()
{
    /* MSP430FR2433 does not have a DMA. */
    /* This function will unlock the FRAM write protection */
    uint8_t state = HWREG8(SYS_BASE + OFS_SYSCFG0_L);
    uint8_t wp = DFWP | PFWP;
    HWREG16(SYS_BASE + OFS_SYSCFG0) = FWPW | (state & ~wp);
}
#endif




#if defined(__MSP430FR2433__)
void Software_Trim()
{
    unsigned int oldDcoTap = 0xffff;
    unsigned int newDcoTap = 0xffff;
    unsigned int newDcoDelta = 0xffff;
    unsigned int bestDcoDelta = 0xffff;
    unsigned int csCtl0Copy = 0;
    unsigned int csCtl1Copy = 0;
    unsigned int csCtl0Read = 0;
    unsigned int csCtl1Read = 0;
    unsigned int dcoFreqTrim = 3;
    unsigned char endLoop = 0;

    do {
        CSCTL0 = 0x100;                         // DCO Tap = 256
        do {
            CSCTL7 &= ~DCOFFG;                  // Clear DCO fault flag
        }while (CSCTL7 & DCOFFG);               // Test DCO fault flag

        __delay_cycles((unsigned int)3000 * MCLK_FREQ_MHZ);// Wait FLL lock status (FLLUNLOCK) to be stable
                                                           // Suggest to wait 24 cycles of divided FLL reference clock
        while((CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)) && ((CSCTL7 & DCOFFG) == 0));

        csCtl0Read = CSCTL0;                   // Read CSCTL0
        csCtl1Read = CSCTL1;                   // Read CSCTL1

        oldDcoTap = newDcoTap;                 // Record DCOTAP value of last time
        newDcoTap = csCtl0Read & 0x01ff;       // Get DCOTAP value of this time
        dcoFreqTrim = (csCtl1Read & 0x0070)>>4;// Get DCOFTRIM value

        if(newDcoTap < 256)                    // DCOTAP < 256
        {
            newDcoDelta = 256 - newDcoTap;     // Delta value between DCPTAP and 256
            if((oldDcoTap != 0xffff) && (oldDcoTap >= 256)) // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim--;
                CSCTL1 = (csCtl1Read & (~(DCOFTRIM0+DCOFTRIM1+DCOFTRIM2))) | (dcoFreqTrim<<4);
            }
        }
        else                                   // DCOTAP >= 256
        {
            newDcoDelta = newDcoTap - 256;     // Delta value between DCPTAP and 256
            if(oldDcoTap < 256)                // DCOTAP cross 256
                endLoop = 1;                   // Stop while loop
            else
            {
                dcoFreqTrim++;
                CSCTL1 = (csCtl1Read & (~(DCOFTRIM0+DCOFTRIM1+DCOFTRIM2))) | (dcoFreqTrim<<4);
            }
        }

        if(newDcoDelta < bestDcoDelta)         // Record DCOTAP closest to 256
        {
            csCtl0Copy = csCtl0Read;
            csCtl1Copy = csCtl1Read;
            bestDcoDelta = newDcoDelta;
        }

    }while(endLoop == 0);                      // Poll until endLoop == 1

    CSCTL0 = csCtl0Copy;                       // Reload locked DCOTAP
    CSCTL1 = csCtl1Copy;                       // Reload locked DCOFTRIM
    while(CSCTL7 & (FLLUNLOCK0 | FLLUNLOCK1)); // Poll until FLL is locked
}
#endif
