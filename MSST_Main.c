/*
 * main.c
 */

#include <MSST_PWM.h>
#include "MSST_GlobalFunctions.h"
#include "F28x_Project.h"
#include "Syncopation_SCI.h"

#include "Syncopation_Data.h"
#include "MSST_PWM.h"

void deadloop();
void CpuTimerInit();
void CpuTimerIsr();

#define CPU_INT_MSEC 20

void main(void) {
	InitSysCtrl();

	EALLOW;
	ClkCfgRegs.LOSPCP.bit.LSPCLKDIV = 0;
	ClkCfgRegs.PERCLKDIVSEL.bit.EPWMCLKDIV = 0;
	EDIS;

    MSSTGpioConfig();
    GpioDataRegs.GPACLEAR.bit.GPIO18 = 1;  // Not prepared yet.
    CPU_LED_BIT = 0;
    DINT;
    InitPieCtrl();
    InterruptInit();

    SCI_Config();
    AdcInit();
    PwmInit();


    PieCtrlRegs.PIECTRL.bit.ENPIE = 1;  // Enable the PIE block
    IER = M_INT1 | M_INT9;

    EINT;

	deadloop();
}


Uint16 log_send_count = 0;

extern float Vac;
extern float Iac;
extern float Vdc;
extern float Idc;
extern Uint16 Prd;
extern Uint16 Duty;

extern Uint16 V_DC;
extern Uint16 I_DC_1;
extern Uint16 I_DC_2;
extern Uint16 I_AC_1;
extern Uint16 I_AC_2;
extern Uint16 LIGHT;

extern Uint16 TEMP_1;
extern Uint16 TEMP_2;
extern Uint16 TEMP_3;
extern Uint16 TEMP_4;

extern float Iac_mag;
extern float Reactive_current;

extern float Grid_Freq;
extern float Grid_Amplitude;

extern Uint16 rect_state;
#pragma CODE_SECTION(deadloop, ".TI.ramfunc");
void deadloop()
{
    while(1)
    {
        DELAY_US(4000);

        SCI_UpdatePacketFloat(0, Grid_Amplitude);
        SCI_UpdatePacketFloat(1, Vdc);
        SCI_UpdatePacketFloat(2, Iac_mag);
        SCI_UpdatePacketFloat(3, Reactive_current);

        SCI_UpdatePacketInt16(0,rect_state);
//        SCI_UpdatePacketInt16(1,I_DC_1);
//        SCI_UpdatePacketInt16(2,I_DC_2);
//        SCI_UpdatePacketInt16(3,I_AC_1);
//        SCI_UpdatePacketInt16(4,I_AC_2);
//        SCI_UpdatePacketInt16(0,LIGHT);
//        SCI_UpdatePacketInt16(0,TEMP_1);
//        SCI_UpdatePacketInt16(0,TEMP_2);
//        SCI_UpdatePacketInt16(0,TEMP_3);
//        SCI_UpdatePacketInt16(0,TEMP_4);
        SCI_SendPacket();

//        DataLog_ISR();
    }
}

void CpuTimerInit()
{
    CpuTimer1Regs.TCR.all = 0x4010;
    CpuTimer1Regs.PRD.all = 200000 * CPU_INT_MSEC;
    EALLOW;
    PieVectTable.TIMER1_INT = &CpuTimerIsr;
    EDIS;

    DELAY_US(1);
//    CpuTimer1Regs.TCR.all = 0x4000;
}

#pragma CODE_SECTION(CpuTimerIsr, ".TI.ramfunc");
__interrupt void CpuTimerIsr()
{
    CPU_LED_TOGGLE = 1;

    CpuTimer1Regs.TCR.bit.TIF = 1;
}
