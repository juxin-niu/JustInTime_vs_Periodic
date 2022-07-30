
#include <driver/uart2target.h>
#include <stdio.h>
#include <string.h>

void uart2target_init()
{
    GPIO_setAsPeripheralModuleFunctionOutputPin(UART_TX_PORT, UART_TX_PIN, UART_TX_FUNCTION);

    EUSCI_A_UART_initParam param = {0};
    param.selectClockSource = EUSCI_A_UART_CLOCKSOURCE_SMCLK;   /* Use SMCLK in 1MHz */
    param.parity = EUSCI_A_UART_NO_PARITY;                      /* No parity check */
    param.msborLsbFirst = EUSCI_A_UART_LSB_FIRST;               /* LSB first */
    param.numberofStopBits = EUSCI_A_UART_ONE_STOP_BIT;         /* 1 stop bit */
    param.uartMode = EUSCI_A_UART_MODE;

    param.clockPrescalar = CLK_PRESCALAR_USED;
    param.firstModReg = FIRST_MODE_REG_USED;
    param.secondModReg = SECOND_MODE_REG_USED;
    param.overSampling = OVER_SAMP_USED;

    if (STATUS_FAIL == EUSCI_A_UART_init(UART_BASEADDR, &param)) {
        while (1){}
    }

    EUSCI_A_UART_enable(UART_BASEADDR);             // Enable UART.
}


static __nv uint8_t int2str_buf[INT2STR_BUFLEN] = {};
static const uint8_t int2str_zero[] = {'0', '\0'};
uint8_t* int2str(uint16_t val) {
    if (val == 0)   return (uint8_t*)int2str_zero;

    int2str_buf[INT2STR_BUFLEN - 1] = 0;
    uint16_t x = INT2STR_BUFLEN - 1;
    while (val > 0) {
        x--;
        int2str_buf[x] = (val % 10) + '0';
        val /= 10;
    }
    return &(int2str_buf[x]);
}

int fputc(int _c, register FILE *_fp)
{
  EUSCI_A_UART_transmitData(UART_BASEADDR, (unsigned char) _c );
  return((unsigned char)_c);
}

int fputs(const char *_ptr, register FILE *_fp)
{
  unsigned int i, len;

  len = strlen(_ptr);

  for(i=0 ; i<len ; i++)
  {
    EUSCI_A_UART_transmitData(UART_BASEADDR, (unsigned char) _ptr[i]);
  }

  return len;
}

