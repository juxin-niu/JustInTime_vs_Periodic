
#ifndef DRIVER_DRIVERLIB_INCLUDE_H_
#define DRIVER_DRIVERLIB_INCLUDE_H_

#include <msp430.h>

#if defined(__MSP430FR5969__) || defined(__MSP430FR5994__)
    #include <lib/driverlib_MSP430FR5xx_6xx/driverlib.h>
#elif defined(__MSP430FR2433__)
    #include <lib/driverlib_MSP430FR2xx_4xx/driverlib.h>
#else
    #error "The model of the development board is not supported."
#endif

#endif /* DRIVER_DRIVERLIB_INCLUDE_H_ */
