#include <testbench/global_declaration.h>
#include <testbench/testbench_api.h>


__nv uint16_t candidate_e;
__nv uint16_t e_number;
__nv uint16_t enc_index;
__nv uint16_t dec_index;

__nv uint16_t public_n;
__nv uint16_t phi_n;
__nv uint16_t sqrt_candidate_e;

__nv uint16_t secret_d[RSA_MSGLENGTH];
__nv uint16_t public_e[RSA_MSGLENGTH];
__nv uint16_t enc_cipher[RSA_MSGLENGTH];
__nv uint16_t plain[RSA_MSGLENGTH];
__nv uint16_t dec_plain[RSA_MSGLENGTH];


void jit_rsa_main()
{
    // Initialization
    uint16_t i;

    public_n = p * q;
    phi_n = (p - 1) * (q - 1);
    candidate_e = p + q - 1;
    e_number = 0;
    enc_index = 0;
    dec_index = 0;

    get_sqrt_e:
    sqrt_candidate_e = RSA_Sqrt16(candidate_e);

    if (RSA_isPrime(candidate_e, sqrt_candidate_e))
        public_e[e_number++] = candidate_e;

    if (e_number < RSA_MSGLENGTH)
    {
        candidate_e += 2;
        goto get_sqrt_e;
    }

    enc_main:
    i = enc_index++;
    enc_cipher[i] = RSA_PowerMod(rsa_msg[i], public_e[i], public_n);

    if (enc_index < RSA_MSGLENGTH)
        goto enc_main;

    calculate_d_main:
    i = dec_index;
    secret_d[i] = RSA_ModInv(public_e[i], phi_n);

    i = dec_index++;
    dec_plain[i] = RSA_PowerMod(enc_cipher[i], secret_d[i], public_n);

    if (dec_index < RSA_MSGLENGTH)
        goto calculate_d_main;

}

