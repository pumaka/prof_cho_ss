#include "main.h"
#include <math.h>

#define MODE_CLEAR	 0x01;
#define MODE_SET	 0x02;
#define MODE_TOGGLE	 0x03;

#define EXT_ON		GpioDataRegs.GPBCLEAR.bit.GPIO63 = 1;
#define EXT_OFF		GpioDataRegs.GPBSET.bit.GPIO63 = 1;
#define EXT_TOGGLE	GpioDataRegs.GPBTOGGLE.bit.GPIO63 = 1;



unsigned int duty_scaled = 0;


/**********************************/
/*             Module             */
float ref_current = 10;
float ref_voltage = 400;
float integration0 = 0;
float integration1 = 0;
float integration2 = 0;
float time_stamp = 1; // time stamp has to be assessed with clock
float fittest_duty = 0.5; // modulation

/*switching duty*/
unsigned int duty_scaled_conv_switch = 0;
unsigned int duty_scaled_inv_switch1 = 0;
unsigned int duty_scaled_inv_switch2 = 0;


void initialize(){
	ref_current = 10;
	ref_voltage = 400;
	integration0 = 0;
	integration1 = 0;
	integration2 = 0;
	time_stamp = 1;
}

float sensed_value_converter(float sensed){
	float actual = 0; // needs to be modified, find the coefficient
	actual = sensed*coefficient;
	return actual;
}

void epwm_activation(float pin_num, float duty){

}

float control_c(float sensed_i_dc){
	//// abbreviation later
	float err = ref_current - sensed_i_dc;
	integration0 += 250*err*time_stamp;
	err = 0.1*err + integration0;
	// err becomes duty
	return 1/err;
	// add epwm modulation function

}

float control_i(float sensed_i_ac, float sensed_v_dc){
	// volt ctrl
	float err = ref_voltage - sensed_v_dc;
	integration1 += 300*err*time_stamp;
	err = -(0.05*err + integration1);
	err = err*sin(wt) // needs to be fixed
	err = err - sensed_i_ac;
	integration2 += 250*err*time_stamp;
	err = 0.02*err + integration2;
	// value stored in err = duty;
	return 1/err;
	// add epwm modulation function and activate the other half as well
}

void mppt_mode(){
	/* leave inverter volt_ref to 400v, and continue with mppt
	 * change the duty of the converter and sense the input values.
	 * caculate power with maximum resolution, but the process should not take long (find the trade off point)
	 */



}
/**************************************/

void stup()
{
	/**/
	//reference
	EALLOW;
	GpioCtrlRegs.GPAMUX1.bit.GPIO8 = 1; // pin 17, epwm reg 5
	GpioCtrlRegs.GPAMUX1.bit.GPIO5 = 1; // epwm 1A
	GpioCtrlRegs.GPAMUX1.bit.GPIO6 = 1; // epwm 1B
	GpioCtrlRegs.GPAMUX1.bit.GPIO7 = 1;  //epwm 2A
	GpioCtrlRegs.GPAMUX1.bit.GPIO10 = 1; //epwm 2B
	GpioCtrlRegs.GPAMUX2.bit.GPIO18 = 1; //epwm 5B
	GpioCtrlRegs.GPAMUX2.bit.GPIO20 = 1; //epwm 6B

	/*what is this?*/
	GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 0;

	GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;
	/**************/
	EDIS;

	EPwm5Regs.TBPRD = 3750;
	EPwm5Regs.TBCTL.bit.CTRMODE = MODE_SET; // updown
	EPwm5Regs.AQCTLA.bit.CAU = MODE_CLEAR;	// 1
	EPwm5Regs.AQCTLA.bit.CAD = MODE_SET;	// 2
	/**/
	
	//sensor gpio declaration needed

	EPwm5Regs.TBPRD = 3750;
	EPwm5Regs.TBCTL.bit.CTRMODE = MODE_SET; // updown
	EPwm5Regs.AQCTLA.bit.CAU = MODE_CLEAR;	// 1
	EPwm5Regs.AQCTLA.bit.CAD = MODE_SET;	// 2

	EPwm6Regs.TBPRD = 3750;
	EPwm6Regs.TBCTL.bit.CTRMODE = MODE_SET; // updown
	EPwm6Regs.AQCTLA.bit.CAU = MODE_CLEAR;	// 1
	EPwm6Regs.AQCTLA.bit.CAD = MODE_SET;	//2

	EPwm1Regs.TBPRD = 3750;
	EPwm1Regs.TBCTL.bit.CTRMODE = MODE_SET; // updown
	EPwm1Regs.AQCTLA.bit.CAU = MODE_CLEAR;	// 1
	EPwm1Regs.AQCTLA.bit.CAD = MODE_SET;	// 2

	EPwm2Regs.TBPRD = 3750;
    	EPwm2Regs.TBCTL.bit.CTRMODE = MODE_SET; // updown
	EPwm2Regs.AQCTLA.bit.CAU = MODE_CLEAR;	// 1
	EPwm2Regs.AQCTLA.bit.CAD = MODE_SET;	// 2
}


void main(void)
{
	InitSysCtrl();
	DINT;
	InitPieCtrl();

	IER = 0x0000;
	IFR = 0x0000;

	InitPieVectTable();

	stup();

	duty_scaled = (EPwm5Regs.TBPRD)/2;  /*inside main function because it took advantage of stup() func*/

	EINT;   // Enable Global interrupt INTM
	ERTM;   // Enable Global realtime interrupt DBGM

	easyDSP_SCI_Init();					// easyDSP setup

	/********************************/
	//       process
	///interrupt ?
	while(1){
		duty_scaled_conv_switch = EPwm5Regs.TBPRD/(control_c(/*sensed_value*/));
		duty_scaled_inv_switch1 = EPwm1Regs.TBPRD/(control_i(/*sensed_value*/));
		duty_scaled_inv_switch2 = -duty_scaled_inv_switch1;
		EPwm5Regs.CMPA.half.CMPA = duty_scaled_conv_switch;
		EPwm6Regs.CMPA.half.CMPA = duty_scaled_conv_switch;
		EPwm1Regs.CMPA.half.CMPA = duty_scaled_inv_switch1;
		EPwm2Regs.CMPA.half.CMPA = duty_scaled_inv_switch2;
	}

	EXT_ON;
}

