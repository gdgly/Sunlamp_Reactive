/*
 * ChbControl.c
 *
 *  Created on: Jan 27, 2019
 *      Author: y3437
 */

#include "F28x_Project.h"
#include "Syncopation_Data.h"

#define TS 2e-5

float Grid_Freq = 0;
float Grid_Angle = 0;
float Grid_Amplitude = 0;

float Reactive_current = 0;



extern float SinglePhasePLL(float Vac, float *Freq, float *Vac_amp);


float Vdc_filter = 0;
float vdc_h1_x1 = 0;
float vdc_h1_x2 = 0;
float vdc_h2_x1 = 0;
float vdc_h2_x2 = 0;
float vdc_h0_x = 160;
float vdc_sogi_k = 1.414;
float vdc_h0_k = 100;

//void v_sogi_k_p(float arg_2, float arg_3)
//{
//    vdc_sogi_k = arg_2;
//    vdc_h0_k = arg_3;
//}

void reactive_current_set(float arg_2)
{
    Reactive_current = arg_2;
    if(Reactive_current > 5)
        Reactive_current = 5;
    if(Reactive_current < -5)
        Reactive_current = -5;
}

#pragma CODE_SECTION(voltage_sogi, ".TI.ramfunc");
float voltage_sogi(float Vdc, float Freq)
{
    float v_omega_T = Freq * TS;
    float v_2omega_T = 2 * Freq * TS;

    float vdc_h1_x1_n;
    float vdc_h1_x2_n;
    float vdc_h2_x1_n;
    float vdc_h2_x2_n;
    float vdc_h0_x_n;

    float v_sogi_error = vdc_sogi_k * (Vdc - vdc_h1_x1 - vdc_h2_x1 - vdc_h0_x);

    vdc_h0_x_n = vdc_h0_x + vdc_h0_k * v_sogi_error * TS;

    vdc_h1_x1_n = __cospuf32(v_omega_T) * vdc_h1_x1 - __sinpuf32(v_omega_T) * vdc_h1_x2 + TS * 377 * v_sogi_error;
    vdc_h1_x2_n = __sinpuf32(v_omega_T) * vdc_h1_x1 + __cospuf32(v_omega_T) * vdc_h1_x2;

    vdc_h2_x1_n = __cospuf32(v_2omega_T) * vdc_h2_x1 - __sinpuf32(v_2omega_T) * vdc_h2_x2 + TS * 377 * 2 * v_sogi_error;
    vdc_h2_x2_n = __sinpuf32(v_2omega_T) * vdc_h2_x1 + __cospuf32(v_2omega_T) * vdc_h2_x2;


    vdc_h1_x1 = vdc_h1_x1_n;
    vdc_h1_x2 = vdc_h1_x2_n;
    vdc_h2_x1 = vdc_h2_x1_n;
    vdc_h2_x2 = vdc_h2_x2_n;
    vdc_h0_x = vdc_h0_x_n;

    return vdc_h0_x;
}

float v_loop_inte = 0;
float v_loop_error = 0;

//float v_loop_kp = 0.06;
//float v_loop_ki = 0.6;

float v_loop_kp = 0.05;
float v_loop_ki = 0.5;

//float v_loop_kp = 0.01;
//float v_loop_ki = 0.01;
//
//void v_loop_pi(float arg_2, float arg_3)
//{
//    v_loop_kp = arg_2;
//    v_loop_ki = arg_3;
//}

#pragma CODE_SECTION(voltage_loop, ".TI.ramfunc");
float voltage_loop(float Vdc_ref, float Vdc_filtered)
{
    v_loop_error = Vdc_ref - Vdc_filtered;

    v_loop_inte += TS * v_loop_error;

    float I_ac_ref = v_loop_kp * v_loop_error + v_loop_ki * v_loop_inte;

    return I_ac_ref;
}

// I_loop_variables
float i_kp = 40;
float i_kr = 100;
//float i_kp = 10;
//float i_kr = 25;
float i_sogi_x1 = 0;
float i_sogi_x2 = 0;
float i_dc_offset_inte = 0;
float i_sogi_error;
float i_omega_h1_T;
// End of I_loop_variables

//void current_loop_pr(float arg_2, float arg_3)
//{
//    i_kp = arg_2;
//    i_kr = arg_3;
//}

#pragma CODE_SECTION(current_loop, ".TI.ramfunc");
float current_loop(float Iac_ref, float Iac, float Freq)
{
    i_omega_h1_T = Freq * TS;

    float i_sogi_h1_x1_n;
    float i_sogi_h1_x2_n;

    i_sogi_error = -Iac_ref + Iac;

    i_sogi_h1_x1_n = __cospuf32(i_omega_h1_T) * i_sogi_x1 - __sinpuf32(i_omega_h1_T) * i_sogi_x2 + TS * 377 * i_sogi_error;
    i_sogi_h1_x2_n = __sinpuf32(i_omega_h1_T) * i_sogi_x1 + __cospuf32(i_omega_h1_T) * i_sogi_x2;

    i_sogi_x1 = i_sogi_h1_x1_n;
    i_sogi_x2 = i_sogi_h1_x2_n;

    float Vac_ref = i_kp * i_sogi_error + i_kr * i_sogi_x1;

    return Vac_ref;
}

float Vdc_ref = 240;
float Iac_ref = 0;
float Iac_mag = 0;
float Vac_ref = 0;

void ResetState()
{
    i_sogi_x1 = 0;
    i_sogi_x2 = 0;
    i_dc_offset_inte = 0;

    v_loop_inte = 0;

    Iac_ref = 0;
    Iac_mag = 0;
    Vac_ref = 0;
}

extern Uint16 rect_state;

Uint16 trigger_state = 0;
Uint16 trigger_count = 0;
Uint16 trigger_limit = 4000;

void Rect_Trigger()
{
    rect_state = 1;
    trigger_state = 1;
    trigger_count = 0;
}

void Rect_on()
{
    if(rect_state == 0)
        rect_state = 1;
    ResetState();
}

void Rect_off()
{
    if(rect_state == 1)
        rect_state = 0;
    ResetState();
}

float SinglePhaseRectifier(float Vac, float Vdc, float Iac)
{
    Grid_Angle = SinglePhasePLL(Vac, &Grid_Freq, &Grid_Amplitude);
    Vdc_filter = voltage_sogi(Vdc, Grid_Freq);

    Vac_ref = 0;

    if(rect_state == 1)
    {
        Iac_mag = voltage_loop(Vdc_ref, Vdc_filter);

        if(Iac_mag > 15)
            Iac_mag = 15;


        Iac_ref = Iac_mag * __cospuf32(Grid_Angle) + Reactive_current * __sinpuf32(Grid_Angle);

        Vac_ref = current_loop(Iac_ref, Iac, Grid_Freq);
    }


//    if(trigger_state == 1)
//    {
//        DataLog_Logging(trigger_count, Vac, Iac_ref, Iac, Vdc);
//
//        trigger_count++;
//        if(trigger_count>=trigger_limit)
//        {
//            rect_state = 0;
//            trigger_state = 0;
//            ResetState();
//            DataLog_StartToSend(trigger_limit);
//        }
//    }
    return (Vac_ref/Vdc);
}
