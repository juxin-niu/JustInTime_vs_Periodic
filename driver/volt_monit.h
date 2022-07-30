
#ifndef DRIVER_VOLT_MONIT_H_
#define DRIVER_VOLT_MONIT_H_

#include <stdint.h>

#if defined(__MSP430FR5994__)
#define ADC_INPUT_SOURCE                ADC12_B_INPUT_A2                    // P1.2 A2, P1SEL1.x = 1, P1SEL0.x = 1
#define ADC_CHANNEL_INPUT_PORT          GPIO_PORT_P1                        //
#define ADC_CHANNEL_INPUT_PIN           GPIO_PIN2                           //
#define ADC_CHANNEL_INPUT_FUNCTION      GPIO_TERNARY_MODULE_FUNCTION        //
#elif defined(__MSP430FR5969__)
#define ADC_INPUT_SOURCE                ADC12_B_INPUT_A7                    // P2.4 A7, P1SEL1.x = 1, P1SEL0.x = 1
#define ADC_CHANNEL_INPUT_PORT          GPIO_PORT_P2                        //
#define ADC_CHANNEL_INPUT_PIN           GPIO_PIN4                           //
#define ADC_CHANNEL_INPUT_FUNCTION      GPIO_TERNARY_MODULE_FUNCTION        //
#endif


#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
#define ADC_TRIGGER_TIMERA_BASE         TIMER_A0_BASE                       // TA0 CCR1 output
#define ADC_TRIGGER_TIMERA_CMPREG       TIMER_A_CAPTURECOMPARE_REGISTER_1   //
#elif defined(__MSP430FR2433__)
#define ADC_TRIGGER_TIMERA_BASE         TIMER_A1_BASE                       // TA1.1B
#define ADC_TRIGGER_TIMERA_CMPREG       TIMER_A_CAPTURECOMPARE_REGISTER_1   //
#endif


void ref_volt_init();

void adc_timer_init(uint16_t timePeriod);

void adc_init();

#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)

#define adc_start   ADC12_B_startConversion(ADC12_B_BASE, ADC12_B_MEMORY_0, ADC12_B_REPEATED_SINGLECHANNEL);\
    Timer_A_startCounter(ADC_TRIGGER_TIMERA_BASE, TIMER_A_UP_MODE)

#define adc_stop    HWREG16(ADC_TRIGGER_TIMERA_BASE + OFS_TAxCTL)  &= ~MC_3; \
    ADC12_B_disable(ADC12_B_BASE) \


#define adc_clear_interrupt     HWREG16(ADC12_B_BASE + OFS_ADC12IFGR0) &= ~(ADC12_B_IFG0)

#define adc_read_memory     HWREG16(ADC12_B_BASE + (OFS_ADC12MEM0 + ADC12_B_MEMORY_0))

#elif defined(__MSP430FR2433__)

#define adc_start   ADC_startConversion(ADC_BASE, ADC_REPEATED_SINGLECHANNEL);\
        Timer_A_startCounter(ADC_TRIGGER_TIMERA_BASE, TIMER_A_UP_MODE)

#define adc_clear_interrupt     HWREG16(ADC_BASE + OFS_ADCIFG) &= ~(ADC_COMPLETED_INTERRUPT_FLAG)

#define adc_read_memory     HWREG16(ADC_BASE + OFS_ADCMEM0)

#endif

#endif /* DRIVER_VOLT_MONIT_H_ */
