#define _CC_

#include "main.h"
#include "math.h"

float duty = 0.0;
#define pi 3.14159265358979323846

void polling_parameter_update()
{

}

void current_controller()
{
	// EXT_ADC Trigger
	GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1;
	GpioDataRegs.GPBSET.bit.GPIO39 = 1;

	delay(150);
	cc_cnt++;


	AD_EXT1 = AD_EXT1_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT1_OFFSET);
	AD_EXT2 = AD_EXT2_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT2_OFFSET);
	AD_EXT3 = AD_EXT3_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT3_OFFSET);
	AD_EXT4 = AD_EXT4_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT4_OFFSET);

	/////////////////////////////
	//contorller
	EPwm5Regs.CMPA.half.CMPA = EPwm5Regs.TBPRD*sin(2*pi*60*Tsamp*counter);
	EPwm1Regs.CMPA.half.CMPA = EPwm1Regs.TBPRD*sin(2*pi*60*Tsamp*counter);
	EPwm2Regs.CMPA.half.CMPA = EPwm2Regs.TBPRD*sin(2*pi*60*Tsamp*counter);

	counter ++;






	////////////////////////////
	/*

	pwm_g1.phase_a_duty_scaled = (int)(((float)pwm_g1.phase_duty_max_scaled)*duty);

	EPwm1Regs.CMPA.half.CMPA = pwm_g1.phase_a_duty_scaled;
	EPwm2Regs.CMPA.half.CMPA = pwm_g1.phase_a_duty_scaled;
	EPwm3Regs.CMPA.half.CMPA = pwm_g1.phase_a_duty_scaled;
	EPwm4Regs.CMPA.half.CMPA = pwm_g1.phase_a_duty_scaled;
	EPwm5Regs.CMPA.half.CMPA = pwm_g1.phase_a_duty_scaled;
	EPwm6Regs.CMPA.half.CMPA = pwm_g1.phase_a_duty_scaled;
*/
    flag_dac = 1;
}


