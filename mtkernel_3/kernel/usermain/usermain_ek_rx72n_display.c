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

#define VPFP		( 6UL)				//   6 Line
#define	HPFP		( 2UL)				//   2 Pixel
#define VPBP		( 8UL)				//   8 Line
#define	HPBP		(37UL)				//  37 Pixel
#define BGPERI_FV	(VPFP+1+VPBP+BGVSIZE_VW+1)	// 288 Line
#define BGPERI_FH	(HPFP+4+HPBP+BGHSIZE_HW+2)	// 525 Pixel
#define BGVSIZE_VP	(VPFP+1+VPBP)			//  15 Line
#define BGVSIZE_VW	(272UL)				// 272 Line
#define BGHSIZE_HP	(HPFP+4+HPBP)			//  43 Pixel
#define BGHSIZE_HW	(480UL)				// 480 Pixel
#define VPWH		(1UL)				//   1 Line
#define	HPWH		(1UL)				//   1 Pixel

#pragma address frmbuf1=0x00800100
UH frmbuf1[272][480];
#pragma address frmbuf2=0x00840100
UH frmbuf2[272][480];

EXPORT void display_tsk(INT stacd, void *exinf)
{
INT i;
	tk_dis_dsp( );						// Dispatch Disable
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( GLCDC ) = 0;					// Enable GLCDC
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	PORT6.PODR.BIT.B7 = 1;					// Set Output Value(Back Light ON)
	PORT6.PDR.BIT.B7 = 1;					// P67 is Output(Back Light Enable)
	PORTB.PODR.BIT.B3 = 1;					// Set Output Value(DISP ON)
	PORTB.PDR.BIT.B3 = 1;					// PB3 is Output(DISP Enable)
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
	GLCDC.PANELCLK.LONG = 0x01100108;			// Use PLL, 8 Division (30MHz)
	GLCDC.PANELCLK.BIT.CLKEN = 1;				// Enable LCD_CLK Output
	while( ! GLCDC.BGMON.BIT.SWRST )  ;			// Wait Software Reset End
	GLCDC.OUTSET.LONG = 0x00002000;				// Set Data Format is RBG(565)
	GLCDC.PANELDTHA.LONG = 0x00020000;
	GLCDC.BGPERI.LONG  = (  BGPERI_FV << 16 ) +  BGPERI_FH;	// FV is 288 Line,  FH is 525 Pixel
	GLCDC.BGSYNC.LONG  = (       VPFP << 16 ) +       HPFP;	// VP is   6 Line,  HP is   2 Pixel
	GLCDC.BGVSIZE.LONG = ( BGVSIZE_VP << 16 ) + BGVSIZE_VW;	// VP is  15 Line,  VW is 272 Line
	GLCDC.BGHSIZE.LONG = ( BGHSIZE_HP << 16 ) + BGHSIZE_HW;	// HP is  43 Pixel, HW is 480 Pixel
//	GLCDC.TCONTIM.LONG =   ( 0 << 16 ) + 0;			// HALF is 1 Pixel, OFFSET is 1 Pixel
	GLCDC.TCONSTVA1.LONG = ( 0 << 16 ) + VPWH;		// VS is   0 Line, VW is   1 Line of STVA
	GLCDC.TCONSTVB1.LONG = ( ( BGVSIZE_VP - VPFP ) << 16 ) + BGVSIZE_VW;
								// VS is   9 Line, VW is 272 Line of STVB
	GLCDC.TCONSTVA2.LONG = ( 1 <<  4 ) + 1;			// LCD_TCON0 is Invert of STVA Signal (VSYNC)
	GLCDC.TCONSTHA1.LONG = ( 0 << 16 ) + HPWH;		// HS is   0 Pixel, HW is   1 Pixel of STHA
	GLCDC.TCONSTHB1.LONG = ( ( BGHSIZE_HP - HPFP ) << 16 ) + BGHSIZE_HW;
								// HS is  41 Pixel, HW is 480 Pixel of STHB
	GLCDC.TCONSTHA2.LONG = ( 1 <<  4 ) + 2;			// LCD_TCON2 is Invert of STHA Signal (HSYNC)
	GLCDC.TCONSTHB2.LONG = ( 0 <<  4 ) + 7;			// LCD_TCON3 is Normal of DE Signal (DE)
	GLCDC.BRIGHT1.LONG =   0x200;				// G Brightness Adjustment is 0
	GLCDC.BRIGHT2.LONG = ( 0x200 << 16 ) + 0x200;		// R, B Brightness Adjustment is 0
	GLCDC.CONTRAST.LONG = ( 0x80 << 16 ) + ( 0x80 << 8 ) + 0x80;	// Contrast Adjustment is All 1.00
	GLCDC.CLKPHASE.LONG = 0x00001000;			// Enable Contrast Adjustment
//	GLCDC.BGCOLOR.LONG = 0x00000000;			// Background Color is Black
//	GLCDC.GR1BASE.LONG = 0x00000000;			// Set GR1 Background Color (Black)
	GLCDC.GR1FLM2 = (UW)frmbuf1;				// Set GR1 Frame Buffer Address
	GLCDC.GR1FLM3.LONG = ( ( BGHSIZE_HW * 2 ) << 16 );	// Set GR1 Macro Line Offset
	GLCDC.GR1FLM5.LONG = ( ( BGVSIZE_VW - 1 ) << 16 ) + BGHSIZE_HW * 2 / 64 - 1;	// Set GR1 LNNUM is 271, DATANUM is 14
//	GLCDC.GR1FLM6.LONG = 0 << 27;				// Set GR1 Frame Buffer Color Format RGB(565)
	GLCDC.GR1AB1.LONG = 0x00000002;				// Set Displays Current (GR1) Graphics
	GLCDC.GR1AB2.LONG = ( ( VPBP + 1 ) << 16 ) + BGVSIZE_VW;// Set GR1 GRCVS is  9 Line,  GRCVW is 272 Line
	GLCDC.GR1AB3.LONG = ( ( HPBP + 4 ) << 16 ) + BGHSIZE_HW;// Set GR1 GRCHS is 41 Pixel, GRCHW is 480 Pixel
	GLCDC.GR1FLMRD.LONG = 0x00000001;			// Enable Reading of GR1 Frame Buffer
//	GLCDC.GR2BASE.LONG = 0x00000000;			// Set GR2 Background Color (Black)
	GLCDC.GR2FLM2 = (UW)frmbuf2;				// Set GR2 Frame Buffer Address
	GLCDC.GR2FLM3.LONG = ( ( BGHSIZE_HW * 2 ) << 16 );	// Set GR2 Macro Line Offset
	GLCDC.GR2FLM5.LONG = ( ( BGVSIZE_VW - 1 ) << 16 ) + BGHSIZE_HW * 2 / 64 - 1;	// Set GR2 LNNUM is 271, DATANUM is 14
//	GLCDC.GR2FLM6.LONG = 0 << 27;				// Set GR2 Frame Buffer Color Format RGB(565)
	GLCDC.GR2AB1.LONG = 0x00001003;				// Set Displays Current (GR2) Graphics
//	GLCDC.GR2AB1.LONG = 0x00000001;				// Set Displays Current (GR2) Graphics
	GLCDC.GR2AB2.LONG = ( ( VPBP + 1 ) << 16 ) + BGVSIZE_VW;// Set GR2 GRCVS is  9 Line,  GRCVW is 272 Line
	GLCDC.GR2AB3.LONG = ( ( HPBP + 4 ) << 16 ) + BGHSIZE_HW;// Set GR2 GRCHS is 41 Pixel, GRCHW is 480 Pixel
	GLCDC.GR2AB4.LONG = ( ( VPBP + 1 ) << 16 ) + BGVSIZE_VW;// Set GR2 ARCVW is  9 Line,  ARCVS is 272 Line
	GLCDC.GR2AB5.LONG = ( ( HPBP + 4 ) << 16 ) + BGHSIZE_HW;// Set GR2 ARCHW is 41 Pixel, ARCHS is 480 Pixel
	GLCDC.GR2FLMRD.LONG = 0x00000001;			// Enable Reading of GR2 Frame Buffer
	GLCDC.BGEN.LONG = 0x00010101;				// Enable Background Generating Block Operation
	tk_dly_tsk( 3000 );					// Wait 3000 ms
	GLCDC.DTCTEN.LONG = 0x00000001;				// Enable Detection of Specified Line Notification
	tm_putstring(" Brightness Adjustment");
	for( i=0x1FF ; 0<=i ; i-- )  {				// Brightness Counter
		if( !( i & 3 ) )
			tm_printf("\r\t\t\t%+5d", i - 0x200 );
		GLCDC.STCLR.LONG = 0x00000001;			// Clear VPOS Flag
		while(   GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Clear
		while( ! GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Set
		GLCDC.BRIGHT1.LONG = i;				// Set G Brightness Adjustment
		GLCDC.BRIGHT2.LONG = ( i << 16 ) + i;		// Set R, B Brightness Adjustment
		GLCDC.OUTVEN.LONG = 0x00000001;			// Reflection Register Values
		tk_dly_tsk( 5 );				// Wait 5ms
	}
	for( i=1 ; 0x400>i ; i++ )  {				// Brightness Counter
		if( !( i & 3 ) )
			tm_printf("\r\t\t\t%+5d", i - 0x200 );
		GLCDC.STCLR.LONG = 0x00000001;			// Clear VPOS Flag
		while(   GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Clear
		while( ! GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Set
		GLCDC.BRIGHT1.LONG = i;				// Set G Brightness Adjustment
		GLCDC.BRIGHT2.LONG = ( i << 16 ) + i;		// Set R, B Brightness Adjustment
		GLCDC.OUTVEN.LONG = 0x00000001;			// Reflection Register Values
		tk_dly_tsk( 5 );				// Wait 5ms
	}
	for( i=0x3FE ; 0x200<=i ; i-- )  {			// Brightness Counter
		if( !( i & 3 ) )
			tm_printf("\r\t\t\t%+5d", i - 0x200 );
		GLCDC.STCLR.LONG = 0x00000001;			// Clear VPOS Flag
		while(   GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Clear
		while( ! GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Set
		GLCDC.BRIGHT1.LONG = i;				// Set G Brightness Adjustment
		GLCDC.BRIGHT2.LONG = ( i << 16 ) + i;		// Set R, B Brightness Adjustment
		GLCDC.OUTVEN.LONG = 0x00000001;			// Reflection Register Values
		tk_dly_tsk( 5 );				// Wait 5ms
	}
	tm_putstring("\n\r Contrast Adjustment");
	for( i=0x7F ; 0<=i ; i-- )  {				// Contrast Counter
		if( !( i & 3 ) )
			tm_printf("\r\t\t\t0.%03d", i * 1000 / 128 );
		GLCDC.STCLR.LONG = 0x00000001;			// Clear VPOS Flag
		while(   GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Clear
		while( ! GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Set
		GLCDC.CONTRAST.LONG = ( i<<16 ) + ( i<<8 ) + i;	// Set Contrast Adjustment
		GLCDC.OUTVEN.LONG = 0x00000001;			// Reflection Register Values
		tk_dly_tsk( 20 );				// Wait 20ms
	}
	for( i=1 ; 0x100>i ; i++ )  {				// Contrast Counter
		if( !( i & 3 ) && i > 0x80 )
			tm_printf("\r\t\t\t1.%03d", ( i - 128 ) * 1000 / 128 );
		else if( !( i & 3 ) && i < 0x80 )
			tm_printf("\r\t\t\t0.%03d", i * 1000 / 128 );
		GLCDC.STCLR.LONG = 0x00000001;			// Clear VPOS Flag
		while(   GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Clear
		while( ! GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Set
		GLCDC.CONTRAST.LONG = ( i<<16 ) + ( i<<8 ) + i;	// Set Contrast Adjustment
		GLCDC.OUTVEN.LONG = 0x00000001;			// Reflection Register Values
		tk_dly_tsk( 20 );				// Wait 20ms
	}
	for( i=0xFE ; 0x80<=i ; i-- )  {			// Brightness Counter
		if( !( i & 3 ) )
			tm_printf("\r\t\t\t1.%03d", ( i - 128 ) * 1000 / 128 );
		GLCDC.STCLR.LONG = 0x00000001;			// Clear VPOS Flag
		while(   GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Clear
		while( ! GLCDC.STMON.BIT.VPOS )  ;		// Wait VPOS Flag is Set
		GLCDC.CONTRAST.LONG = ( i<<16 ) + ( i<<8 ) + i;	// Set Contrast Adjustment
		GLCDC.OUTVEN.LONG = 0x00000001;			// Reflection Register Values
		tk_dly_tsk( 20 );				// Wait 20ms
	}
	tm_putstring("\r\n Alpha Blending\r\n");
	GLCDC.GR2AB6.LONG = 0x00010000;				// Set Alpha Blending Parameter
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