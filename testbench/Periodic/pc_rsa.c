
#include <testbench/global_declaration.h>
#include <testbench/Periodic_scheduler.h>
#include <testbench/testbench_api.h>

__SHARED_VAR(
uint16_t candidate_e;
uint16_t e_number;
uint16_t enc_index;
uint16_t dec_index;

uint16_t public_n;
uint16_t phi_n;
uint16_t sqrt_candidate_e;

uint16_t secret_d[RSA_MSGLENGTH];
uint16_t public_e[RSA_MSGLENGTH];
uint16_t enc_cipher[RSA_MSGLENGTH];
uint16_t dec_plain[RSA_MSGLENGTH];
)

static __nv bool      first_run = 1;
static __nv uint16_t  status = 0;
static const uint16_t global_war_size = 4;
static __nv uint16_t  backup_buf[4] = {};


static const bool backup_needed[] = {
    false, false, true, true, false, true
};

void pc_rsa_main()
{
    // Local variables
    uint16_t i;

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
    case 1: goto get_e_sqrt;
    case 2: goto e_verify;
    case 3: goto enc_main;
    case 4: goto calculate_d_main;
    case 5: goto dec_main;
    }

    // Tasks
    // =================================================================
    __BUILDIN_TASK_BOUNDARY(0, init);
    __GET(public_n) = p * q;
    __GET(phi_n) = (p - 1) * (q - 1);
    __GET(candidate_e) = p + q - 1;
    __GET(e_number) = 0;
    __GET(enc_index) = 0;
    __GET(dec_index) = 0;

    __NEXT(1, get_e_sqrt);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(1, get_e_sqrt);
    __GET(sqrt_candidate_e) = RSA_Sqrt16(__GET(candidate_e));
    __NEXT(2, e_verify);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(2, e_verify);
    if (RSA_isPrime(__GET(candidate_e), __GET(sqrt_candidate_e)))
       __GET(public_e[__GET(e_number)++]) = __GET(candidate_e);

    if (__GET(e_number) < RSA_MSGLENGTH)
    {
       __GET(candidate_e) += 2;
       __NEXT(1, get_e_sqrt);
    }

    __NEXT(3, enc_main);


    // =================================================================
    __BUILDIN_TASK_BOUNDARY(3, enc_main);
    i = __GET(enc_index)++;
    __GET(enc_cipher[i]) =
           RSA_PowerMod(rsa_msg[i], __GET(public_e[i]), __GET(public_n));

    if (__GET(enc_index) < RSA_MSGLENGTH) {
        __NEXT(3, enc_main);
    }
    else {
        __NEXT(4, calculate_d_main);
    }

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(4, calculate_d_main);
    i = __GET(dec_index);
    __GET(secret_d[i]) = RSA_ModInv(__GET(public_e[i]), __GET(phi_n));
    __NEXT(5, dec_main);

    // =================================================================
    __BUILDIN_TASK_BOUNDARY(5, dec_main);
    i = __GET(dec_index)++;
    __GET(dec_plain[i]) =
           RSA_PowerMod(__GET(enc_cipher[i]), __GET(secret_d[i]), __GET(public_n));

    if (__GET(dec_index) < RSA_MSGLENGTH) {
        __NEXT(4, calculate_d_main);
    }
    else {
        __FINISH;
    }

}
