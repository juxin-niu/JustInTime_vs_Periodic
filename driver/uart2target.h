
#ifndef DRIVER_UART2TARGET_H_
#define DRIVER_UART2TARGET_H_

#include <driver/driverlib_include.h>
#include <driver/nv.h>
#include <driver/target.h>
#include <msp430.h>
#include <stdint.h>
#include <stdbool.h>

#define INT2STR_BUFLEN 20

#if defined(__MSP430FR5994__)
    #define UART_TX_PORT            GPIO_PORT_P6
    #define UART_TX_PIN             GPIO_PIN0
    #define UART_TX_FUNCTION        GPIO_PRIMARY_MODULE_FUNCTION
    #define UART_BASEADDR           EUSCI_A3_BASE
#elif defined(__MSP430FR5969__)
    #define UART_TX_PORT            GPIO_PORT_P2
    #define UART_TX_PIN             GPIO_PIN5
    #define UART_TX_FUNCTION        GPIO_SECONDARY_MODULE_FUNCTION
    #define UART_BASEADDR           EUSCI_A1_BASE
#elif defined(__MSP430FR2433__)
    #define UART_TX_PORT            GPIO_PORT_P2
    #define UART_TX_PIN             GPIO_PIN6
    #define UART_TX_FUNCTION        GPIO_PRIMARY_MODULE_FUNCTION
    #define UART_BASEADDR           EUSCI_A1_BASE
#else
#error "ERROR: UNSUPPORTED MSP TARGET!"
#endif


#define CLK_PRESCALAR_USED          8
#define FIRST_MODE_REG_USED         0
#define SECOND_MODE_REG_USED        214
#define OVER_SAMP_USED              0

void uart2target_init();
uint8_t* int2str(uint16_t val);

#endif /* DRIVER_UART2TARGET_H_ */
