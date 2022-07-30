#ifndef IMC_ANALYSIS_MISC_H_
#define IMC_ANALYSIS_MISC_H_

// =================================================================== //
//                                 LED                                 //
#if defined(__MSP430FR5994__)
#define turn_on_red_led             P1OUT |= BIT0
#define turn_off_red_led            P1OUT &= ~BIT0
#define turn_on_green_led           P1OUT |= BIT1
#define turn_off_green_led          P1OUT &= ~BIT1
#elif defined(__MSP430FR5969__)
#define turn_on_red_led             P4OUT |= BIT6
#define turn_off_red_led            P4OUT &= ~BIT6
#define turn_on_green_led           P1OUT |= BIT0
#define turn_off_green_led          P1OUT &= ~BIT0
#elif defined(__MSP430FR2433__)
#define turn_on_red_led             P1OUT |= BIT0
#define turn_off_red_led            P1OUT &= ~BIT0
#define turn_on_green_led           P1OUT |= BIT1
#define turn_off_green_led          P1OUT &= ~BIT1
#endif


// =================================================================== //
//                                BUTTON                               //
#if defined(__MSP430FR5994__)
#define left_button_init                GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P5, GPIO_PIN6)
#define check_button_press              (GPIO_getInputPinValue(GPIO_PORT_P5, GPIO_PIN6) == GPIO_INPUT_PIN_LOW)
#define left_button_disable             GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN6); \
                                        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN6)
#elif defined(__MSP430FR5969__)
#define left_button_init                GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN5)
#define check_button_press              (GPIO_getInputPinValue(GPIO_PORT_P4, GPIO_PIN5) == GPIO_INPUT_PIN_LOW)
#define left_button_disable             GPIO_setAsOutputPin(GPIO_PORT_P4, GPIO_PIN5); \
                                        GPIO_setOutputLowOnPin(GPIO_PORT_P4, GPIO_PIN5)
#elif defined(__MSP430FR2433__)
#define left_button_init                GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN3)
#define check_button_press              (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN3) == GPIO_INPUT_PIN_LOW)
#define left_button_disable             GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN3); \
                                        GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN3)
#endif

#endif /* IMC_ANALYSIS_MISC_H_ */
