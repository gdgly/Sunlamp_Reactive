/*
 * MSST_ADC.c
 *
 *  Created on: Dec 14, 2016
 *      Author: Yang Lei
 */

#include "MSST_PWM.h"
#include "F28x_Project.h"
#include "MSST_GlobalFunctions.h"
#include "Syncopation_SCI.h"
#include "Syncopation_Data.h"

Uint16 V_DC;
Uint16 I_DC_1;
Uint16 I_DC_2;
Uint16 I_AC_1;
Uint16 I_AC_2;
Uint16 LIGHT;

Uint16 TEMP_1;
Uint16 TEMP_2;
Uint16 TEMP_3;
Uint16 TEMP_4;

void ControlLoop(void);
void Adc_A_Init();
void Adc_B_Init();


void AdcInit()
{
    Adc_A_Init();
    Adc_B_Init();

    PieCtrlRegs.PIEIER1.bit.INTx2 = 1;  // ADC-B interrupt 1
    EALLOW;
    PieVectTable.ADCB1_INT = &ControlLoop;
    EDIS;
}

void Adc_A_Init()
{
    EALLOW;
    AdcaRegs.ADCCTL1.bit.ADCPWDNZ = 1;  // ADC-A power up
    AdcaRegs.ADCCTL1.bit.INTPULSEPOS = 1; // Interrupt position at the end of conversion
    AdcaRegs.ADCCTL2.bit.PRESCALE = 2; // ADC Clock = SYSCLK / 2;
    AdcaRegs.ADCCTL2.bit.RESOLUTION = 0; // 12-bit resolution
    AdcaRegs.ADCCTL2.bit.SIGNALMODE = 0; // Single-ended signal mode
    AdcaRegs.ADCBURSTCTL.bit.BURSTEN = 0; // Disable burst mode
    AdcaRegs.ADCINTFLGCLR.all = 15; // Clear all the interrupt flags
    AdcaRegs.ADCINTOVFCLR.all = 15; // Clear all the interrupt overflow flags
    AdcaRegs.ADCINTSEL1N2.all = 0; //  Enable ADCINT1 and it is triggered by EOC3, disable ADCINT2
    AdcaRegs.ADCINTSEL3N4.all = 0; //  Disable ADCINT3 and ADCINT4
    AdcaRegs.ADCSOCPRICTL.all = 0; // Round robin control. Conversion starts from SOC0.
    AdcaRegs.ADCINTSOCSEL1.all = 0; // ADC interrupt doesn't trigger any SOC
    AdcaRegs.ADCINTSOCSEL2.all = 0; // ADC interrupt doesn't trigger any SOC

    AdcaRegs.ADCSOC0CTL.bit.TRIGSEL = 5; // SOC-0 software start
    AdcaRegs.ADCSOC0CTL.bit.CHSEL = 2; // SOC-0 convert channel 2, which is ADC_A2 pin
    AdcaRegs.ADCSOC0CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcaRegs.ADCSOC1CTL.bit.TRIGSEL = 5; // SOC-1 software start
    AdcaRegs.ADCSOC1CTL.bit.CHSEL = 3; // SOC-1 convert channel 3, which is ADC_A3 pin
    AdcaRegs.ADCSOC1CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcaRegs.ADCSOC2CTL.bit.TRIGSEL = 5; // SOC-2 software start
    AdcaRegs.ADCSOC2CTL.bit.CHSEL = 4; // SOC-2 convert channel 4, which is ADC_A4 pin
    AdcaRegs.ADCSOC2CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcaRegs.ADCSOC3CTL.bit.TRIGSEL = 5; // SOC-3 software start
    AdcaRegs.ADCSOC3CTL.bit.CHSEL = 5; // SOC-3 convert channel 5, which is ADC_A5 pin
    AdcaRegs.ADCSOC3CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles
    EDIS;
}

void Adc_B_Init()
{
    EALLOW;
    AdcbRegs.ADCCTL1.bit.ADCPWDNZ = 1;  // ADC-A power up
    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1; // Interrupt position at the end of conversion
    AdcbRegs.ADCCTL2.bit.PRESCALE = 2; // ADC Clock = SYSCLK / 2;
    AdcbRegs.ADCCTL2.bit.RESOLUTION = 0; // 12-bit resolution
    AdcbRegs.ADCCTL2.bit.SIGNALMODE = 0; // Single-ended signal mode
    AdcbRegs.ADCBURSTCTL.bit.BURSTEN = 0; // Disable burst mode
    AdcbRegs.ADCINTFLGCLR.all = 15; // Clear all the interrupt flags
    AdcbRegs.ADCINTOVFCLR.all = 15; // Clear all the interrupt overflow flags
    AdcbRegs.ADCINTSEL1N2.all = 0x0025; //  Enable ADCINT1 and it is triggered by EOC5, disable ADCINT2
    AdcbRegs.ADCINTSEL3N4.all = 0; //  Disable ADCINT3 and ADCINT4
    AdcbRegs.ADCSOCPRICTL.all = 0; // Round robin control. Conversion starts from SOC0.
    AdcbRegs.ADCINTSOCSEL1.all = 0; // ADC interrupt doesn't trigger any SOC
    AdcbRegs.ADCINTSOCSEL2.all = 0; // ADC interrupt doesn't trigger any SOC

    AdcbRegs.ADCSOC0CTL.bit.TRIGSEL = 5; // SOC-0 EPWM7 SOCA
    AdcbRegs.ADCSOC0CTL.bit.CHSEL = 2; // SOC-0 convert channel 2, which is ADC_B2 pin
    AdcbRegs.ADCSOC0CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcbRegs.ADCSOC1CTL.bit.TRIGSEL = 5; // SOC-1 EPWM7 SOCA
    AdcbRegs.ADCSOC1CTL.bit.CHSEL = 3; // SOC-1 convert channel 3, which is ADC_B3 pin
    AdcbRegs.ADCSOC1CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcbRegs.ADCSOC2CTL.bit.TRIGSEL = 5; // SOC-2 EPWM7 SOCA
    AdcbRegs.ADCSOC2CTL.bit.CHSEL = 4; // SOC-2 convert channel 4, which is ADC_B4 pin
    AdcbRegs.ADCSOC2CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcbRegs.ADCSOC3CTL.bit.TRIGSEL = 5; // SOC-3 EPWM7 SOCA
    AdcbRegs.ADCSOC3CTL.bit.CHSEL = 5; // SOC-3 convert channel 5, which is ADC_B5 pin
    AdcbRegs.ADCSOC3CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcbRegs.ADCSOC4CTL.bit.TRIGSEL = 5; // SOC-4 EPWM7 SOCA
    AdcbRegs.ADCSOC4CTL.bit.CHSEL = 14; // SOC-3 convert channel 14, which is ADC_14 pin
    AdcbRegs.ADCSOC4CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcbRegs.ADCSOC5CTL.bit.TRIGSEL = 5; // SOC-5 EPWM7 SOCA
    AdcbRegs.ADCSOC5CTL.bit.CHSEL = 15; // SOC-3 convert channel 15, which is ADC_15 pin
    AdcbRegs.ADCSOC5CTL.bit.ACQPS = 19; // Sample window is 49+1 clock cycles

    AdcbRegs.ADCCTL1.bit.INTPULSEPOS = 1;    // ADCINT1 trips after AdcResults latch
    AdcbRegs.ADCINTSEL1N2.bit.INT1E     = 1;    // Enabled ADCINT1
    AdcbRegs.ADCINTSEL1N2.bit.INT1CONT  = 0;    // Disable ADCINT1 Continuous mode
    AdcbRegs.ADCINTSEL1N2.bit.INT1SEL   = 5;    // setup EOC3 to trigger ADCINT1 to fire

    EDIS;
}

// Feedback signals
float Vac = 0;
float Iac = 0;
float Vdc = 0;
float duty = 0;
Uint16 rect_state = 0;

extern float SinglePhaseRectifier(float Vac, float Vdc_sec, float Iac);

Uint16 led_count = 0;

#pragma CODE_SECTION(ControlLoop, ".TI.ramfunc");
__interrupt void ControlLoop(void)
{
    V_DC = AdcbResultRegs.ADCRESULT1;
    I_DC_1 = AdcbResultRegs.ADCRESULT2;
    I_DC_2 = AdcbResultRegs.ADCRESULT0;
    I_AC_1 = AdcbResultRegs.ADCRESULT4;
    I_AC_2 = AdcbResultRegs.ADCRESULT5;
    LIGHT = AdcbResultRegs.ADCRESULT3;

    TEMP_1 = AdcaResultRegs.ADCRESULT0;
    TEMP_2 = AdcaResultRegs.ADCRESULT1;
    TEMP_3 = AdcaResultRegs.ADCRESULT2;
    TEMP_4 = AdcaResultRegs.ADCRESULT3;

    Vac = -0.1297458799 * I_DC_1 + 239.8857157878;
    Iac = -0.0226524031 * I_AC_1 + 51.1086755868;
    Vdc = 0.2703198055 * V_DC - 0.2205776208;


    if((Iac>15) || (Iac<-15))
    {
        rect_state = 0;
        Pwm_DIS();
    }

    if(Vdc > 400)
    {
        rect_state = 0;
        Pwm_DIS();
    }

    if(rect_state == 1)
        Pwm_EN();
    else
        Pwm_DIS();


    duty = SinglePhaseRectifier(Vac, Vdc, Iac);
    RectDuty_SET(duty);


    led_count++;
    if(led_count>=20000)
    {
        led_count = 0;
        CPU_LED_TOGGLE = 1;
    }

    AdcbRegs.ADCINTFLGCLR.bit.ADCINT1 = 1;	//Clear ADCINT1 flag reinitialize for next SOC
	PieCtrlRegs.PIEACK.bit.ACK1 = 1;
}
