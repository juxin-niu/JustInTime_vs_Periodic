
#include <driver/driverlib_include.h>
#include <driver/backup_and_recovery.h>
#include <driver/nv.h>
#include <driver/volt_monit.h>
#include <global_config.h>
#include <stdint.h>

__nv uint8_t backup_error_flag = false;
__nv uint8_t recovery_needed = 0;
__nv uint8_t system_in_lpm = 0;

#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
#pragma vector = ADC12_VECTOR
#elif defined(__MSP430FR2433__)
#pragma vector=ADC_VECTOR
#endif
__interrupt void ADC12_ISR(void)
{
    uint16_t mem_read = adc_read_memory;

#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
    uint16_t volt = (uint16_t)( ((uint32_t)mem_read * 890) >> 10 );
#elif defined(__MSP430FR2433__)
    uint16_t volt = (uint16_t)( ((uint32_t)mem_read * 4500) >> 10 );
#endif

    if (system_in_lpm == 1 && volt > RECOVERY_VOLT_THRESHOLD) {
        if (recovery_needed == 0) {
            recovery_needed = 1;
            goto safe_exit_and_running;
        }
        recovery();
    }

    else if (system_in_lpm == 0 && volt < BACKUP_VOLT_THRESHOLD) {
        backup_error_flag = true;
        backup();
        asm("  nop");   // Recovery here.

        /* The branch the program enters after the snapshot is taken */
        if (backup_error_flag == true) {
            backup_error_flag = false;
            goto sleep;
        }
        /* The branch the program enters after the system is recovery */
        else
            goto safe_exit_and_running;
    }

    else goto default_exit;

    safe_exit_and_running:
    system_in_lpm = 0;
    __bic_SR_register_on_exit(LPM1_bits);
    // Go on to dafault_exit.

    default_exit:
    adc_clear_interrupt;
    return;

    sleep:
    system_in_lpm = 1;
    adc_clear_interrupt;
    __bis_SR_register(GIE | LPM1_bits);     // LPM1 and nested interrupt enabled.
}
