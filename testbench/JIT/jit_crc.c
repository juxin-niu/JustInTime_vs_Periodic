#include <testbench/global_declaration.h>
#include <testbench/testbench_api.h>

__nv uint16_t SW_Results;
__nv uint16_t cnt;

void jit_crc_main()
{

    cnt = 0;
    SW_Results = CRC_INIT;

    run:
    SW_Results = CRCheck_CCITT_Update(SW_Results, CRC_Input[cnt] & 0xFF);
    SW_Results = CRCheck_CCITT_Update(SW_Results, (CRC_Input[cnt] >> 8) & 0xFF);
    ++cnt;

    if(cnt < CRC_LENGTH)
        goto run;

    return;
}
