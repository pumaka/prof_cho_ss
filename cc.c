#define _CC_

#include "main.h"
#include "math.h"

#define RELAY_OFF GpioDataRegs.GPBCLEAR.bit.GPIO40 = 1;
#define RELAY_ON GpioDataRegs.GPBSET.bit.GPIO40 = 1;

float duty = 1.0;

#define pi 3.14159265358979323846
#define sqrt_2 1.41421356237

float Vqs = 0.0, Vds = 0.0, Vde = 0.0, Vqe = 0.0;
float thetar_est = 0.0, Vac = 0.0, Wr_est = 0.0, Wr_fb_PLL = 0.0, x_pre_APF = 0.0;
float coff_b_APF = 0.0, coff_b_LPF_Em = 0.0, Em_est_LPF = 0.0, Em_est = 0.0, Err_PLL = 0.0, Kp_pll = 15;
float coff_a_APF = 0.0, coff_a_LPF_Em = 0.0;
float f_APF = 60.0, f_LPF_Em = 1.0, w_APF = 0.0, w_LPF_Em = 0.0;
float sin_thetar_grid=0.0, thetar_grid=0.0, sin_thetar =0.0, cos_thetar_grid=0.0;
float theta=0.0, ref_freq=0.0, sinwav=0.0;
float duty_ref=0.0, duty_ref_ne=0.0;
float I_inv_falut=0.0, I_inv_falut_flag=0.0;
int relay_state = 0;
int cc=0;
float V_dc_fault=0.0, V_dc_fault_flag=0.0;
float I_ref=0.0, I_err=0.0, kiI_con_1=0.0, ki_I=15, kpI_con_1=0.0, PI_con=0.0, PI_con_ne=0.0, Int_err=0.0, kp_I=0.005, I_ref_in=0.0, I_ref_in_con=0.0;
float kpI_con = 0.0;
float V_conv_in = 0.0, kpV_con = 0.0, kiV_con = 0.0, Vnt_err = 0.0, V_err = 0.0;
float I_dc_fault = 0.0, I_dc_fault_flag=0.0;
float Kp_V_con = 0.0, Ki_V_con = 0.0;
float I_con_err = 0.0, Int_con_err = 0.0, iV_con_1 = 0.0, iV_con_2 = 0.0;
///////////////////////////////
float ref_current = 0; //10
float ref_voltage = 0; //400
float integration0 = 0; //boost converter current controller
float integration1 = 0; //invertor voltage controller
float integration2 = 0;// time stamp has to be assessed with clock //invertor current controller
float power = 0; // modulation
float err_c[4] = {0};
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


float V_con = 0;
float I_inv = 0;
float I_con = 0;
float Vac_1 = 0;


void polling_parameter_update()
{
	// APF parameters for PLL
	w_APF = 2*PI*f_APF;
	coff_a_APF = (1 - w_APF*Tsamp)/(1 + w_APF*Tsamp);
	coff_b_APF = 1/(1 + w_APF*Tsamp);

	// LPF parameters for PLL
	w_LPF_Em = 2*PI*f_LPF_Em;
	coff_a_LPF_Em = (w_LPF_Em*Tsamp)/(1 + w_LPF_Em*Tsamp);
	coff_b_LPF_Em = 1/(1 + w_LPF_Em*Tsamp);
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

	V_con = AD_EXT3;
	I_inv = AD_EXT2;
	I_con = AD_EXT1;
	Vac = AD_EXT4;


	I_inv_falut = abs(I_inv);
	V_dc_fault = V_con;
	I_dc_fault = I_con;
	if(I_inv_falut > 20 || V_dc_fault > 450 || I_dc_fault > 7)
	{
		cc=0;
		pwm_gating_ena=0;
		I_inv_falut_flag=I_inv_falut;
		V_dc_fault_flag=V_dc_fault;
		I_dc_fault_flag=I_dc_fault;
	}
	/////////////////////////////
	//contorller

	/*
	//alpha_beta_generation
	cos_thetar_grid = cos(thetar_grid);
	sin_thetar_grid = sin(thetar_grid);

	// PLL
	Vqs = Vac;
	Vds = coff_a_APF*Vqs - coff_b_APF*x_pre_APF + coff_b_APF*Vds;
	x_pre_APF = Vqs;

	Vde = cos_thetar_grid*Vds + sin_thetar_grid*Vqs;
	Vqe = -sin_thetar_grid*Vds + cos_thetar_grid*Vqs;

	Em_est = sqrt(Vde*Vde + Vqe*Vqe);
	Em_est_LPF = coff_a_LPF_Em*Em_est + coff_b_LPF_Em*Em_est_LPF + 0.00001;

	Err_PLL = Vqe / Em_est_LPF;
	Wr_fb_PLL = Err_PLL * Kp_pll;
	Wr_est = Wr_fb_PLL + 376.99104;
	thetar_grid = BOUND_PI(thetar_grid + Wr_est*Tsamp);
	 */
	//sin_thetar_grid 사용
	theta += TWO_PI*Tsamp*ref_freq;
	if(theta>=TWO_PI)
	{
		theta-=theta;
	}
	sinwav = sin(theta);
	//I_ref_in=I_ref*sinwav*1.414;
	//EPwm5Regs.CMPA.half.CMPA = (int)(((float)pwm_g1.phase_duty_max_scaled)*duty);

/*
		err_c[1] = ref_current - I_con;
		integration0 += 125*(err_c[0]+err_c[1])*Tsamp; /// 0.5*err*250*Tsamp
		err_c[1] = 0.1*err_c[1] + integration0;
		err_c[0] = err_c[1];
*/
	switch(cc){
		case 0:
			pwm_gating_ena = 0;
			integration0 = 0;
			break;
		case 1:
			pwm_gating_ena = 1;
			I_con_err = I_ref_in_con - I_con;
			kpI_con = 0.1 * I_con_err);
			Int_con_err += (I_con_er*Tsamp);
			if (Int_con_err >= 1){
				Int_con_err = 1;
			}
			else if (Int_con_err <= -1){
				Int_con_err = -1;
			}
			iV_con_1 = 250 * Int_con_err;
			iV_con_2 = kpI_con + iV_con_1;
			if (iV_con_2 >= 1){
				iV_con_2 = 1;
			}
			else if (iV_con_2 <= -1){
				iV_con_2 = -1;
			}
				V_err = -(V_conv_in - V_con);
			kpV_con = Kp_V_con * V_err; // 0.05
			Vnt_err += (V_err * Tsamp);
			if (Vnt_err >= 1){
				Vnt_err = 1;
			}
			else if (Vnt_err <= -1){
				Vnt_err = -1;
			}
			kiV_con = Ki_V_con * Vnt_err; // 300
			I_ref_in = (kpV_con + kiV_con)*sinwav*1.414;
			I_err = I_ref_in - I_inv;
			kpI_con_1 = 0.02 * I_err;
			Int_err += (I_err*Tsamp);
			if (Int_err >= 1){
				Int_err = 1;
			}
			else if (Int_err <= -1){
				Int_err = -1;
			}
			kiI_con_1 = 250 * Int_err;
			PI_con = kpI_con_1 + kiI_con_1;
			PI_con = (PI_con>=1)?1:(PI_con<=-1)?-1:PI_con;
			PI_con_ne=-PI_con;
			EPwm5Regs.CMPA.half.CMPA = (int)(((float)pwm_g1.phase_duty_max_scaled)*(1-err_c[3]));
			EPwm1Regs.CMPA.half.CMPA = (int)(((float)pwm_g1.phase_duty_half_scaled*PI_con_ne+pwm_g1.phase_duty_half_scaled));
			EPwm2Regs.CMPA.half.CMPA = (int)(((float)pwm_g1.phase_duty_half_scaled*PI_con+pwm_g1.phase_duty_half_scaled));
			break;
		}
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
}

