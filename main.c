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
float power = 0; // modulation
float err_c[2] = {0};
float err_i1[2] = {0};
float err_i2[2] = {0};
float temp[2] = {0};

/*switching duty*/
unsigned int duty_scaled_conv_switch = 0;
unsigned int duty_scaled_inv_switch1 = 0;
unsigned int duty_scaled_inv_switch2 = 0;

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

	GpioCtrlRegs.GPBMUX1.bit.GPIO42 = 0; //adcina0
	GpioCtrlRegs.GPBMUX1.bit.GPIO41 = 0;
	GpioCtrlRegs.GPBMUX1.bit.GPIO40 = 0;
	GpioCtrlRegs.GPBMUX1.bit.GPIO39 = 0;

	/*what is this?*/
	GpioCtrlRegs.GPBMUX2.bit.GPIO63 = 0;

	GpioCtrlRegs.GPBDIR.bit.GPIO63 = 1;
	/**************/
	EDIS;

	//sensor gpio declaration needed


	EPwm5Regs.TBPRD = 3750;
	EPwm5Regs.TBCTL.bit.CTRMODE = MODE_SET; // updown
	EPwm5Regs.AQCTLA.bit.CAU = MODE_CLEAR;	// 1
	EPwm5Regs.AQCTLA.bit.CAD = MODE_SET;	// 2

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

	EINT;   // Enable Global interrupt INTM
	ERTM;   // Enable Global realtime interrupt DBGM

	easyDSP_SCI_Init();					// easyDSP setup

	/********************************/
	//       process
	///interrupt ?

	sense_conv_v
	sense_conv_i
	for(float duty = 0; duty < 1; duty+= 0.0001){
		EPwm5Regs.CMPA.half.CMPA = (EPwm5Regs.TBPRD*duty);
		time.sleep(2);
		if ((sense_conv_v*sense_conv_i) >= temp[0]){
			temp[0] = sense_conv_v*sense_conv_i;
		    temp[1] = sense_conv_i;
		}
	}
	ref_current = temp[1];

	while(1){
		err_c[0] = err_c[1];
		err_c[1] = ref_current - sensed_i_dc;
		integration0 += 125*(err_c[0]+err_c[1])*time_stamp; /// 0.5*err*250*time_stamp
		err_c[1] = 0.1*err_c[1] + integration0;

		err_i1[0] = err_i1[1];
		err_i1[1] = ref_voltage - sensed_v_dc;
		integration1 += 150*(err_i1[0]+err_i1[1])*time_stamp;
		err_i1[1] = -(0.05*err_i1[1] + integration1);
		err_i2[1] = err_i1[1]*sin(wt); // needs to be fixed
		err_i2[0] = err_i2[1];
		err_i2[1] = err - sensed_i_ac;
		integration2 += 125*(err_i2[0]+err_i2[1])*time_stamp;
		err_i2[1] = 0.02*err_i2[1] + integration2;

		duty_scaled_conv_switch = EPwm5Regs.TBPRD*(err_c[1]);
		duty_scaled_inv_switch1 = EPwm1Regs.TBPRD*(err_i2[1]);
		duty_scaled_inv_switch2 = -duty_scaled_inv_switch1;
		EPwm5Regs.CMPA.half.CMPA = duty_scaled_conv_switch;
		EPwm1Regs.CMPA.half.CMPA = duty_scaled_inv_switch1;
		EPwm2Regs.CMPA.half.CMPA = duty_scaled_inv_switch2;
	}

	EXT_ON;
}

