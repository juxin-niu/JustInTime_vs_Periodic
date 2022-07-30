
#include <main.hpp>

__nv uint8_t task_success_flag = false;
__nv uint8_t running_flag = 0;

__nv uint16_t current_testbench = 0;
__nv uint16_t state = TESTBENCH_READY;

extern __nv uint8_t backup_error_flag;
extern __nv uint8_t recovery_needed;
extern __nv uint8_t system_in_lpm;

void JIT_only();
void Periodic_only();
void Adaptive();

int main()
{
    ref_volt_init();
    adc_timer_init(VOLT_MONITOR_INTERVAL);
    adc_init();
    power_on_init();
    clock_sys_init();
    dma_init();
    uart2target_init();

    if (running_flag == 0) {
        turn_on_green_led;
        left_button_init;
        while (!check_button_press) {/* empty */}
        left_button_disable;
        turn_off_green_led;
        running_flag = 1;
    }

    if (backup_error_flag == true) {
        turn_on_red_led;
        LPM1;
    }

    if (task_success_flag == true) { /* TODO: do something after success. */ }

    // TODO: run testbench
    JIT_only();
    Periodic_only();
    Adaptive();
}

void JIT_only() {
    adc_start;                              // Start ADC and its Timer A
    JIT_SYSTEM_INTO_SLEEP;                  // System Enter LPM1.
    backup_error_flag = true;               // Backup not available

    do {
        current_testbench = 0;
        while (current_testbench < TESTBENCH_LIST_SIZE) {
            jit_run_testbench(current_testbench);
            current_testbench++;
        }
    } while (1);

    // task_success_flag = 1;
    // adc_stop;
}


void Periodic_only() {
    do {
        while (state != TESTBENCH_FINISH) {
            periodic_run_testbench(current_testbench, &state);
        }
        current_testbench++;
        if (current_testbench >= TESTBENCH_LIST_SIZE) current_testbench = 0;
        state = TESTBENCH_READY;
    } while (1);

    // task_success_flag = 1;
}

enum{ USE_JIT, USE_PC };
const uint16_t testbench_ty[TESTBENCH_LIST_SIZE] = {
USE_PC, USE_PC, USE_PC, USE_PC, USE_JIT, USE_JIT, USE_PC, USE_JIT,
// AR   BC      CEM     CRC     CUCKOO   DIJKSTRA RSA     SORT
};

void Adaptive() {
    do {
        if (current_testbench >= TESTBENCH_LIST_SIZE) current_testbench = 0;
        switch(testbench_ty[current_testbench]) {
        case USE_JIT: {
            if (state == TESTBENCH_READY) {
                adc_clear_interrupt;
                backup_error_flag = false;
                recovery_needed = 0;
                system_in_lpm =0;
                state = TESTBENCH_RUNNING;
            }
            adc_start;
            JIT_SYSTEM_INTO_SLEEP;
            backup_error_flag = true;
            jit_run_testbench(current_testbench);
            __bic_SR_register(GIE);
            state = TESTBENCH_READY;
            current_testbench++;
            adc_stop;
        }
            break;
        case USE_PC: {
            while (state != TESTBENCH_FINISH) {
                periodic_run_testbench(current_testbench, &state);
            }
            state = TESTBENCH_READY;
            current_testbench++;
        }
            break;
        } /* End of switch */
    } while (1);
}
