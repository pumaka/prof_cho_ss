// inside Init_ADC()

        EPwm5Regs.TBPRD = 3750;
	EPwm5Regs.TBCTL.bit.CTRMODE = 0x02; // updown
	EPwm5Regs.AQCTLA.bit.CAU = 0x01;	// 1
	EPwm5Regs.AQCTLA.bit.CAD = 0x02;	// 2

	EPwm2Regs.TBPRD = 3750;
	EPwm2Regs.TBCTL.bit.CTRMODE = 0x02; // updown
	EPwm2Regs.AQCTLA.bit.CAU = 0x01;	// 1
	EPwm2Regs.AQCTLA.bit.CAD = 0x02;	// 2

	EPwm3Regs.TBPRD = 3750;
	EPwm3Regs.TBCTL.bit.CTRMODE = 0x02; // updown
	EPwm3Regs.AQCTLA.bit.CAU = 0x01;	// 1
	EPwm3Regs.AQCTLA.bit.CAD = 0x02;	// 2
