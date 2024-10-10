/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usermain.c (usermain)
 *	User Main
 */

#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "iodefine.h"

EXPORT void display_tsk(INT stacd, void *exinf);

#define VP		(2UL)			//   2 Line
#define	HP		(2UL)			//   2 Pixel
#define VPM		(2UL)			//   2 Line
#define	HPM		(10UL)			//  10 Pixel
#define BGPERI_FV	(VP+1+VPM+BGVSIZE_VW+1)	// 278 Line
#define BGPERI_FH	(HP+4+HPM+BGHSIZE_HW+2)	// 498 Pixel
#define BGVSIZE_VP	(VP+1+VPM)		//   5 Line
#define BGVSIZE_VW	(272UL)			// 272 Line
#define BGHSIZE_HP	(HP+4+HPM)		//  16 Pixel
#define BGHSIZE_HW	(480UL)			// 480 Pixel

#pragma address frmbuf1=0x00800100
UH frmbuf1[272][480];
#pragma address frmbuf2=0x00840100
UH frmbuf2[272][480];

EXPORT void display_tsk(INT stacd, void *exinf)
{
	tk_dis_dsp( );						// Dispatch Disable
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( GLCDC ) = 0;					// Enable GLCDC
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	PORT6.PODR.BIT.B7 = 1;					// Set Output Value(Back Light ON)
	PORT6.PDR.BIT.B7 = 1;					// P67 is Output(Back Light Enable)
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
	MPC.PE7PFS.BYTE = 0x25;					// PE7 is LCD_DATA9-B
	MPC.PE6PFS.BYTE = 0x25;					// PE6 is LCD_DATA10-B
	MPC.PE5PFS.BYTE = 0x25;					// PE5 is LCD_DATA11-B
	MPC.PE4PFS.BYTE = 0x25;					// PE4 is LCD_DATA12-B
	MPC.PE3PFS.BYTE = 0x25;					// PE3 is LCD_DATA13-B
	MPC.PE2PFS.BYTE = 0x25;					// PE2 is LCD_DATA14-B
	MPC.PE1PFS.BYTE = 0x25;					// PE1 is LCD_DATA15-B
	MPC.PB5PFS.BYTE = 0x25;					// PB5 is LCD_CLK-B
	MPC.PB4PFS.BYTE = 0x25;					// PB4 is LCD_TCON0-B
	MPC.PB2PFS.BYTE = 0x25;					// PB2 is LCD_TCON2-B
	MPC.PB1PFS.BYTE = 0x25;					// PB1 is LCD_TCON3-B
	MPC.PB0PFS.BYTE = 0x25;					// PB0 is LCD_DATA0-B
	MPC.PA7PFS.BYTE = 0x25;					// PA7 is LCD_DATA1-B
	MPC.PA6PFS.BYTE = 0x25;					// PA6 is LCD_DATA2-B
	MPC.PA5PFS.BYTE = 0x25;					// PA5 is LCD_DATA3-B
	MPC.PA4PFS.BYTE = 0x25;					// PA4 is LCD_DATA4-B
	MPC.PA3PFS.BYTE = 0x25;					// PA3 is LCD_DATA5-B
	MPC.PA2PFS.BYTE = 0x25;					// PA2 is LCD_DATA6-B
	MPC.PA1PFS.BYTE = 0x25;					// PA1 is LCD_DATA7-B
	MPC.PA0PFS.BYTE = 0x25;					// PA0 is LCD_DATA8-B
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	PORTE.PMR.BYTE |= 0xFE;					// PE7-PE1 is Peripheral Pin
	PORTA.PMR.BYTE = 0xFF;					// PA7-PA0 is Peripheral Pin
	PORTB.PMR.BYTE |= 0x37;					// PB5,PB4,PB2-PB0 is Peripheral Pin
	tk_ena_dsp( );						// Dispatch Enable
	GLCDC.BGEN.LONG = 0x00010000;				// Software Reset
	GLCDC.PANELCLK.LONG = 0x01100118;			// Use PLL, 24 Division
	GLCDC.PANELCLK.BIT.CLKEN = 1;				// Enable LCD_CLK Output
	while( ! GLCDC.BGMON.BIT.SWRST )  ;			// Wait Software Reset End
#ifdef __LIT
	GLCDC.OUTSET.LONG = 0x00002000;				// Set Data Format is RBG(565), Little Endian
#else
	GLCDC.OUTSET.LONG = 0x10002000;				// Set Data Format is RBG(565), Big Endian
#endif	/* __LIT */
	GLCDC.PANELDTHA.LONG = 0x00020000;
	GLCDC.BGPERI.LONG  = ( BGPERI_FV  << 16 ) + BGPERI_FH;	// FV is 278 Line,  FH is 498 Pixel
	GLCDC.BGSYNC.LONG  = (        VP  << 16 ) +        HP;	// VP is   2 Line,  HP is   2 Pixel
	GLCDC.BGVSIZE.LONG = ( BGVSIZE_VP << 16 ) + BGVSIZE_VW;	// VP is   5 Line,  VW is 272 Line
	GLCDC.BGHSIZE.LONG = ( BGHSIZE_HP << 16 ) + BGHSIZE_HW;	// HP is  16 Pixel, HW is 480 Pixel
//	GLCDC.BGCOLOR.LONG = 0x00000000;			// Background Color is Black
//	GLCDC.TCONTIM.LONG =   (      0  << 16 ) + 0;		// HALF is 1 Pixel, OFFSET is 1 Pixel
	GLCDC.TCONSTVA1.LONG = (      0  << 16 ) + 1;		// VS is   0 Line, VW is   1 Line of STVA
	GLCDC.TCONSTVB1.LONG = ( (VPM+1) << 16 ) + BGVSIZE_VW;	// VS is   3 Line, VW is 272 Line of STVB
	GLCDC.TCONSTVA2.LONG = (      1  <<  4 ) + 0;		// LCD_TCON0 is Invert of STVA Signal
//	GLCDC.TCONSTVA2.LONG = (      0  <<  4 ) + 0;		// LCD_TCON1 is Normal of STVA Signal
	GLCDC.TCONSTHA1.LONG = (      0  << 16 ) + 1;		// HS is   0 Pixel, HW is   1 Pixel of STHA
	GLCDC.TCONSTHB1.LONG = ( (HPM+1) << 16 ) + BGHSIZE_HW;	// HS is   3 Pixel, HW is 480 Pixel of STHB
	GLCDC.TCONSTHA2.LONG = (      1  <<  4 ) + 2;		// LCD_TCON2 is Invert of STHA Signal
	GLCDC.TCONSTHB2.LONG = (      0  <<  4 ) + 7;		// LCD_TCON3 is Normal of DE
	GLCDC.BRIGHT1.LONG =   0x200;				// G Brightness Adjustment is 0
	GLCDC.BRIGHT2.LONG = ( 0x200 << 16 ) + 0x200;		// R, B Brightness Adjustment is 0
	GLCDC.CONTRAST.LONG = ( 0x80 << 16 ) + ( 0x80 << 8 ) + 0x80;	// Contrast Adjustment is All 1.00
	GLCDC.CLKPHASE.LONG = 0x00001000;			// Enable Contrast Adjustment
//	GLCDC.GR1BASE.LONG = 0x00000000;			// Set GR1 Background Color (Black)
	GLCDC.GR1FLM2 = (UW)frmbuf1;				// Set GR1 Frame Buffer Address
	GLCDC.GR1FLM3.LONG = ( ( BGHSIZE_HW * 2 ) << 16 );	// Set GR1 Macro Line Offset
	GLCDC.GR1FLM5.LONG = ( ( BGVSIZE_VW - 1 ) << 16 ) + BGHSIZE_HW * 2 / 64 - 1;	// Set GR1 LNNUM is 271, DATANUM is 14
	GLCDC.GR1FLM6.LONG = 0x00000001;			// Set GR1 Frame Buffer Color Format RGB(565)
	GLCDC.GR1AB1.LONG = 0x00000002;				// Set Displays Current (GR1) Graphics
	GLCDC.GR1AB2.LONG = ( ( VPM + 1 ) << 16 ) + BGVSIZE_VW;	// Set GR1 GRCVS is  3 Line,  GRCVW is 272 Line
	GLCDC.GR1AB3.LONG = ( ( HPM + 1 ) << 16 ) + BGHSIZE_HW;	// Set GR1 GRCHS is 11 Pixel, GRCHW is 480 Pixel
	GLCDC.GR1FLMRD.LONG = 0x00000001;			// Enable Reading of GR1 Frame Buffer
//	GLCDC.GR2BASE.LONG = 0x00000000;			// Set GR2 Background Color (Black)
	GLCDC.GR2FLM2 = (UW)frmbuf2;				// Set GR2 Frame Buffer Address
	GLCDC.GR2FLM3.LONG = ( ( BGHSIZE_HW * 2 ) << 16 );	// Set GR2 Macro Line Offset
	GLCDC.GR2FLM5.LONG = ( ( BGVSIZE_VW - 1 ) << 16 ) + BGHSIZE_HW * 2 / 64 - 1;	// Set GR2 LNNUM is 271, DATANUM is 14
	GLCDC.GR2FLM6.LONG = 0x00000001;			// Set GR2 Frame Buffer Color Format RGB(565)
	GLCDC.GR2AB1.LONG = 0x00001003;				// Set Displays Current (GR2) Graphics
//	GLCDC.GR2AB1.LONG = 0x00000001;				// Set Displays Current (GR2) Graphics
	GLCDC.GR2AB2.LONG = ( ( VPM + 1 ) << 16 ) + BGVSIZE_VW;	// Set GR2 GRCVS is  3 Line,  GRCVW is 272 Line
	GLCDC.GR2AB3.LONG = ( ( HPM + 1 ) << 16 ) + BGHSIZE_HW;	// Set GR2 GRCHS is 11 Pixel, GRCHW is 480 Pixel
	GLCDC.GR2AB4.LONG = ( ( VPM + 1 ) << 16 ) + BGVSIZE_VW;	// Set GR2 ARCVW is  3 Line,  ARCVS is 272 Line
	GLCDC.GR2AB5.LONG = ( ( HPM + 1 ) << 16 ) + BGHSIZE_HW;	// Set GR2 ARCHW is 11 Pixel, ARCHS is 480 Pixel
	GLCDC.GR2FLMRD.LONG = 0x00000001;			// Enable Reading of GR2 Frame Buffer
	GLCDC.BGEN.LONG = 0x00010101;				// Enable Background Generating Block Operation
	tk_dly_tsk( 3000 );					// Wait 3000 ms
	GLCDC.GR2AB6.LONG = 0x00010001;				// Set Alpha Blending Parameter
	while( 1 )  {
		GLCDC.BGEN.LONG = 0x00010101;			// Enable Alpha Blending
		while( ! GLCDC.GR2MON.BIT.ARCST )  ;		// Wait Alpha Blending Started
		while(   GLCDC.GR2MON.BIT.ARCST )  ;		// Wait Alpha Blending Stopped
		tk_slp_tsk( 3000 );				// Wait 3000 ms
		GLCDC.GR2AB6.LONG ^= 0x01000000;		// Change Increase or Decrease Parameter
	}
}

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
ID objid;

	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task =  display_tsk;				// Set Task Function Address
	strcpy( t_ctsk.dsname, "display" );			// Set Object Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create TFT Display Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) != E_OK )			// Start TFT Display Task
		goto ERROR;
	while( 1 )  tk_slp_tsk(TMO_FEVR);			// Task Waiting
ERROR:
	return 0;
}