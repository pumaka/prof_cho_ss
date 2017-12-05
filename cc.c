#define _CC_

#include "main.h"


#define pi 3.14159265358979323846
#define sqrt_2 1.41421356237

#define Tsamp					(float)50.0e-6

float duty = 0.0;

void polling_parameter_update()
{
}

///////////////////////////////
float ref_current = 10;
float ref_voltage = 400;
float integration0 = 0;
float integration1 = 0;
float integration2 = 0;// time stamp has to be assessed with clock
float power = 0; // modulation
float err_c[2] = {0};
float err_i1[2] = {0};
float err_i2[2] = {0};
float temp[2] = {0};

/*switching duty*/
unsigned int duty_scaled_conv_switch = 0;
unsigned int duty_scaled_inv_switch1 = 0;
unsigned int duty_scaled_inv_switch2 = 0;


float sense_conv_v = 0;
float sense_conv_i = 0;
float sense_conv_end_v = 0;
float sense_inv_i = 0;
////////////////////////////

void current_controller()
{
	// EXT_ADC Trigger
	GpioDataRegs.GPBCLEAR.bit.GPIO39 = 1;
	GpioDataRegs.GPBSET.bit.GPIO39 = 1;

	delay(150);
	cc_cnt++;

	AD_EXT1 = AD_EXT1_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT1_OFFSET);// masked value multiplied by 4
	AD_EXT2 = AD_EXT2_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT2_OFFSET);// & bit operation with 0x00003FFF
	AD_EXT3 = AD_EXT3_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT3_OFFSET);//
	AD_EXT4 = AD_EXT4_GAIN*((float)((0x00003FFF&(*(int *)0x00004000))<<2) - AD_EXT4_OFFSET);

	/////////////////////////////
	//contorller

	//inverter control
	/*
	float duty1 = 0;
	for(duty1 = 0; duty1 <= 1; duty1 += 0.001){
		EPwm5Regs.CMPA.half.CMPA = (EPwm5Regs.TBPRD*duty);
		delay(150);
		sense_conv_v = AD_EXT4;
		sense_conv_i = AD_EXT1;
		if ((sense_conv_v*sense_conv_i) >= temp[0]){
			temp[0] = sense_conv_v*sense_conv_i;
		    temp[1] = sense_conv_i;
		}
	}
	ref_current = temp[1];

	CpuTimer0Regs.TCR.bit.TIE = 1;
	CpuTimer0Regs.TCR.bit.TRB = 1;
	CpuTimer0Regs.TCR.bit.TSS = 1;// prd/ tcr/ tim/ tpr/ tprh/ rsvd1
	PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

	EALLOW;
	PieVecTable.TINT0 = &TINT0_Service_Routine;
	EDIS;
	*/



	sense_conv_i = AD_EXT1
	sense_conv_end_v = AD_EXT2
	sense_inv_i = AD_EXT3
	err_c[0] = err_c[1];
	err_c[1] = ref_current - sense_conv_i;
	integration0 += 125*(err_c[0]+err_c[1])*Tsamp; /// 0.5*err*250*Tsamp
    	err_c[1] = 0.1*err_c[1] + integration0;
	err_i1[0] = err_i1[1];
	err_i1[1] = ref_voltage - sense_conv_end_v;
	integration1 += 150*(err_i1[0]+err_i1[1])*Tsamp;
	err_i1[1] = -(0.05*err_i1[1] + integration1);
	err_i2[1] = err_i1[1]*sqrt_2*sin(2*pi*60*cc_cnt*Tsamp); // needs to be fixed
	err_i2[0] = err_i2[1];
	err_i2[1] = err_i2[1] - sense_inv_i;
	integration2 += 125*(err_i2[0]+err_i2[1])*Tsamp;
	err_i2[1] = 0.02*err_i2[1] + integration2;
	duty_scaled_conv_switch = EPwm5Regs.TBPRD*(1-err_c[1]);
	duty_scaled_inv_switch1 = EPwm1Regs.TBPRD*(err_i2[1]);
	duty_scaled_inv_switch2 = duty_scaled_inv_switch1;
	EPwm5Regs.CMPA.half.CMPA = duty_scaled_conv_switch;
	EPwm1Regs.CMPA.half.CMPA = duty_scaled_inv_switch1;
	EPwm2Regs.CMPA.half.CMPA = duty_scaled_inv_switch2;

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



