
#include <driver/driverlib_include.h>
#include <driver/volt_monit.h>

void ref_volt_init()
{
#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
    Ref_A_setReferenceVoltage(REF_A_BASE, REF_A_VREF1_2V);
#elif defined(__MSP430FR2433__)
    PMM_enableInternalReference();
#endif
}

void adc_timer_init(uint16_t timePeriod)
{
    // MSP5xxx and MSP2xxx share the same TimerA API.

    Timer_A_initUpModeParam timerAInitParam = {0};
    timerAInitParam.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    timerAInitParam.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    timerAInitParam.startTimer = false;
    timerAInitParam.timerClear = TIMER_A_DO_CLEAR;
    timerAInitParam.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    timerAInitParam.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    timerAInitParam.timerPeriod = timePeriod;   // ADC interval

    Timer_A_initUpMode(ADC_TRIGGER_TIMERA_BASE, &timerAInitParam);

    Timer_A_initCompareModeParam compareModeParam = {0};
    compareModeParam.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    compareModeParam.compareOutputMode = TIMER_A_OUTPUTMODE_TOGGLE_SET;
    compareModeParam.compareRegister = ADC_TRIGGER_TIMERA_CMPREG;
    compareModeParam.compareValue = timePeriod >> 1;

    Timer_A_initCompareMode(ADC_TRIGGER_TIMERA_BASE, &compareModeParam);

}

void adc_init()
{
#if defined(__MSP430FR5994__) || defined(__MSP430FR5969__)
    GPIO_setAsPeripheralModuleFunctionOutputPin(ADC_CHANNEL_INPUT_PORT,
                                                ADC_CHANNEL_INPUT_PIN,
                                                ADC_CHANNEL_INPUT_FUNCTION);

    ADC12_B_initParam initParam = {0};
    initParam.clockSourceDivider = ADC12_B_CLOCKDIVIDER_1;
    initParam.clockSourcePredivider = ADC12_B_CLOCKPREDIVIDER__1;
    initParam.clockSourceSelect = ADC12_B_CLOCKSOURCE_ADC12OSC;
    initParam.internalChannelMap = ADC12_B_NOINTCH;
    initParam.sampleHoldSignalSourceSelect = ADC12_B_SAMPLEHOLDSOURCE_1;     // TA0 CCR1 output

    ADC12_B_init(ADC12_B_BASE, &initParam);
    ADC12_B_enable(ADC12_B_BASE);
    ADC12_B_setupSamplingTimer(ADC12_B_BASE,
                               ADC12_B_CYCLEHOLD_32_CYCLES,
                               ADC12_B_CYCLEHOLD_32_CYCLES,
                               ADC12_B_MULTIPLESAMPLESDISABLE
                               );

    ADC12_B_enableInterrupt(ADC12_B_BASE,
                            ADC12_B_IE0,
                            0,
                            0);

    ADC12_B_configureMemoryParam configureMemoryParam = {0};
    configureMemoryParam.memoryBufferControlIndex = ADC12_B_MEMORY_0;
    configureMemoryParam.refVoltageSourceSelect = ADC12_B_VREFPOS_INTBUF_VREFNEG_VSS;
    configureMemoryParam.endOfSequence = ADC12_B_NOTENDOFSEQUENCE;
    configureMemoryParam.windowComparatorSelect = ADC12_B_WINDOW_COMPARATOR_DISABLE;
    configureMemoryParam.differentialModeSelect = ADC12_B_DIFFERENTIAL_MODE_DISABLE;
    configureMemoryParam.inputSourceSelect = ADC_INPUT_SOURCE;

    ADC12_B_configureMemory(ADC12_B_BASE, &configureMemoryParam);

#elif defined(__MSP430FR2433__)
    ADC_init (ADC_BASE,
              ADC_SAMPLEHOLDSOURCE_2,   // TA1.1B
              ADC_CLOCKSOURCE_ADCOSC,
              ADC_CLOCKDIVIDER_1);

    ADC_enable (ADC_BASE);

    ADC_setupSamplingTimer (ADC_BASE,
                            ADC_CYCLEHOLD_32_CYCLES,
                            ADC_MULTIPLESAMPLESDISABLE);

    ADC_enableInterrupt (ADC_BASE, ADC_COMPLETED_INTERRUPT);

    ADC_configureMemory (ADC_BASE,
                         ADC_INPUT_A7,      // P1.7 A7 Analog in.
                         ADC_VREFPOS_INT,
                         ADC_VREFNEG_AVSS);

#else
#error "ERROR: UNSUPPORTED MSP TARGET!"
#endif
}
