
; $Id: 8096.h,v 1.1 1993/05/28 04:40:51 jaw Exp jaw $
; 8096/196 include file
; defines all SFRs and bits

#ifndef _8096_H
#define _8096_H

; useful ops.

; 	set and clear bit masks
#define SBIT(x)		(1<<(x))
#define CBIT(x)		(~(1<<(x)))




.enum r0
.enum ad_result		= 2
.enum ad_command	= 2
.enum hsi_mode
.enum hso_time
.enum hsi_time		= 4
.enum hsi_status	= 6
.enum hso_command	= 6
.enum sbuf
.enum imask
.enum ipend
.enum timer1
.enum watchdog		= 0xa
.enum timer2		= 0xc
.enum baud_rate		= 0xe
.enum ioport0		= 0xe
.enum ioport1
.enum ioport2
.enum sp_stat
.enum sp_con		= 0x11
.enum ioc0		= 0x15
.enum ios0		= 0x15
.enum ios1
.enum ioc1		= 0x16
.enum pwm0_control
.enum sp

.enum ioport3		= 0x1ffe
.enum ioport4

#if defined(80196) || defined(196) || defined(MCS196) || defined(mcs196)
; 80196 specificals

	.enum ioc2	= 0xb,
	.enum ios2	= 0x17
	.enum ipend1	= 0x12
	.enum imask1
	.enum wsr
	.enum ad_time	= 3
	.enum ptssel
	.enum ptssrv	= 6
	.enum ioc3	= 0xc
	.enum t2capture	= 0xc
	.enum pwm1_control = 0x16
	.enum pwm2_control
#endif 196 specific

; bit orders

	/* imask/ipend bits */
.enum	INT_TOV = 0	/* timer overflow */
.enum	INT_ADC		/* adc done */
.enum	INT_HSI		/* hsi data available */
.enum	INT_HSO		/* hso pin */
.enum	INT_HSI_0	/* hsi.0 pin */
.enum	INT_TIMER	/* software timer */
.enum	INT_SIO		/* serial i/o */
.enum	INT_EXT		/* ext int */


	/* imask1/ipend1 bits */
.enum	INT_TI = 0 	/* serial xmit */
.enum	INT_RI 		/* serial rcv */
.enum	INT_HSI_4 	/* >= 4 entries in hsi fifo */
.enum	INT_T2_CAP 	/* t2 capture */
.enum	INT_T2_OV 	/* t2 overflow */
.enum	INT_EXT_ 	/* extint pin */
.enum	INT_FIFO 	/* hsio fifo full */
.enum	INT_NMI		/* nmi */

	/* ioc0 bits */
.enum	IOC0_HSI_0 = 0 		/* hsi.0 enable */
.enum	IOC0_T2_R_EACH_WRITE 	/* t2 reset each write */
.enum	IOC0_HSI_1 		/* hsi.1 enable */
.enum	IOC0_T2_R_EXT 		/* t2 external reset enable */
.enum	IOC0_HSI_2 		/* hsi.2 enable */
.enum	IOC0_T2_R_HSI 		/* t2 reset source is hsi.0 / t2rst' */
.enum	IOC0_HSI_3 		/* hsi.3 enable */
.enum	IOC0_T2_SRC		/* t2 clock source is hsi.1 / t2clk' */

	/* ioc1 bits */
.enum	IOC1_PWM = 0 		/* select pwm / p2.5' */
.enum	IOC1_EXTI_ACH7 		/* extint is ach7 / extint' */
.enum	IOC1_T1_OV 		/* t1 overflow int enable */
.enum	IOC1_T2_OV 		/* t2 overflow int enable */
.enum	IOC1_HSO_4 		/* hso.4 output enable */
.enum	IOC1_TXD 		/* select txd / p2.0' */
.enum	IOC1_HSO_5 		/* hso.5 enable */
.enum	IOC1_HSI_FIFO		/* hsi int on fifo full / hold reg loaded; */

	/* ioc2 bits */
.enum	IOC2_T2_FAST = 0 		/* incr t2 fast enable (once per state time) */
.enum	IOC2_T2_UP_DOWN_ENABLE 		/* enable t2 as up/down */
.enum	IOC2_PWM_SLOW 			/* enable divide by 2 on pwm */
.enum	IOC2_NO_SH 			/* disable sample and hold */
.enum	IOC2_AD_CLK_PRESCALE_DISABLE 	/* disable clock prescaler (for slow xtal) */
.enum	IOC2_T2_ALT_INT 		/* enable t2 int at $8000 */
.enum	IOC2_CAM_LOCK_ENABLE 		/* enable lockable cam feature */
.enum	IOC2_CAM_CLEAR			/* clear entire cam */

	/* ios0 bits */
.enum	IOS0_HSO_0 = 0 	/* current state of each hso pin (0-5) */
.enum	IOS0_HSO_1 
.enum	IOS0_HSO_2 
.enum	IOS0_HSO_3 
.enum	IOS0_HSO_4 
.enum	IOS0_HSO_5 
.enum	IOS0_CAM 	/* cam or hold reg is full */
.enum	IOS0_HSO_REG	/* hold reg is full */

	/* ios1 bits */
.enum	IOS1_ST0 = 0 	/* software timer 0 expired */
.enum	IOS1_ST1 	/* software timer 1 expired */
.enum	IOS1_ST2 	/* software timer 2 expired */
.enum	IOS1_ST3 	/* software timer 3 expired */
.enum	IOS1_T2_OV 	/* t2 overflowed */
.enum 	IOS1_T1_OV 	/* t1 overflowed */
.enum	IOS1_HSI_FIFO 	/* hsi fifo is full */
.enum	IOS1_HSI_REG	/* hsi hold reg data is available */

	/* ios2 bits */
.enum	IOS2_HSO_0 = 0 	/* hso event occured for pin  hso.0 */
.enum	IOS2_HSO_1 	/* hso event occured for pin  hso.1 */
.enum	IOS2_HSO_2 	/* hso event occured for pin  hso.2 */
.enum	IOS2_HSO_3 	/* hso event occured for pin  hso.3 */
.enum	IOS2_HSO_4 	/* hso event occured for pin  hso.4 */
.enum	IOS2_HSO_5 	/* hso event occured for pin  hso.5 */
.enum	IOS2_HC_14 	/* hso event cmd14 (t2 reset)  occured */
.enum	IOS2_HC_15 	/* hso event cmd15 (start adc) occured */
.enum	IOS2_T2_RST = 6 
.enum	IOS2_ADC_STRT = 7


	/* ccb bits */
.enum	CCB_PWR_DN = 0 	/* enable powerdown mode */
.enum	CCB_BUS_16 	/* bus size 16 bit / 8 bit' */
.enum	CCB_WR 		/* write strobe mode wr / wr[lh]' */
.enum	CCB_ALE 	/* ale / adv' */
.enum	CCB_IRC0 	/* internal ready control */
.enum	CCB_IRC1 	/* 00 - limit to 1 wait state 
			   01 -          2
			   10 -          3
			   11 - no limit		*/
.enum	CCB_LOC0 	/* lock mode */
.enum	CCB_LOC1

	/* spcon bits */
.enum	SP_M0 = 0 	/* mode */
.enum	SP_M1 		/* 01 is std; */
.enum	SP_PEN 		/* parity enable */
.enum	SP_REN 		/* enable rcv */
.enum	SP_TB8		/* set 9th bit (for modes 2 3) */

	/* spstat bits */
.enum	SP_OE = 2 	/* output overrun */
.enum	SP_TXE 		/* xmitter empty */
.enum	SP_FE 		/* framing error */
.enum	SP_TI 		/* xmit indicator */
.enum	SP_RI 		/* rcv indicator */
.enum	SP_RPE		/* rcv parity error */

	/* hso command bits */
.enum	HSOC_CH0 
.enum	HSOC_CH1 
.enum	HSOC_CH2 
.enum	HSOC_CH3 
.enum	HSOC_INT 	/* cause an interupt */
.enum	HSOC_SET 	/* set/clear hso line */
.enum	HSOC_T2 	/* base timing on t2/ t1' */
.enum	HSOC_CAM_LOCK	/* lock event in cam */
	/* ch* is 4bit # representing channel:
		0-5	hso 0-5
		6	hso 0 1
		7	hso 2 3
		8-B	software timers
		C D	unflagged events
		E	reset t2
		F	start A/D
	*/


	/* ad command bits */
.enum	ADC_GO = 3	/* start adc now */

	/* ad result bits */
.enum	ADR_STAT = 3	/* conversion in progress / idle' */



	

	
#endif !_8096_H
