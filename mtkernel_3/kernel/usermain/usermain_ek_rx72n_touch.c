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
#include "dev_siic.h"
#include "iodefine.h"

EXPORT void irq4_hdr(UINT intno);
EXPORT void touch_tsk(INT stacd, void *exinf);

typedef enum { TOUCH_TSK, OBJ_KIND_NUM } OBJ_KIND;
EXPORT ID ObjID[OBJ_KIND_NUM];					// IDÉeÅ[ÉuÉã

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
T_DINT t_dint;
ID objid;

	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task =  touch_tsk;				// Set Task Function Address
	strcpy( t_ctsk.dsname, "touch" );			// Set Object Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Touch Sensor Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) != E_OK )			// Start Touch Sensor Task
		goto ERROR;
	ObjID[TOUCH_TSK] = objid;				// Save Touch Task ID
	t_dint.intatr = TA_HLNG;				// Set Interrupt Handler Attribute
	t_dint.inthdr = irq4_hdr;				// Set Interrupt Handler Address
	if( tk_def_int( VECT( ICU, IRQ4 ), &t_dint ) != E_OK )	// Defined Interrupt Handler
		goto ERROR;
	while( 1 )  tk_slp_tsk(TMO_FEVR);			// Task Waiting
ERROR:
	return 0;
}

EXPORT void touch_tsk(INT stacd, void *exinf)
{
ID dd;
UB num, buf[4];
SZ asize;
INT i, x, y;

	dd = tk_opn_dev( "siicg", TD_UPDATE );				// Open Simple IIC Driver
	PORT6.PODR.BIT.B6 = 1;						// Output VAlue is High
	PORT6.PDR.BIT.B6 = 1;						// P66 is Output (Wakuep Signal)
	ICU.IRQCR[4].BYTE = 0x04;					// IRQ4 is Falling Edge
	tk_dis_dsp( );							// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;						// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;						// PmnPFS Write Enable
	MPC.P34PFS.BYTE = 0x40;						// P34 is IRQ4 Pin
	MPC.PWPR.BYTE = 0x80;						// Write Disable
	tk_ena_dsp( );							// Dispatch Enable
	PORT3.PMR.BIT.B4 = 1;						// P34 is Peripheral Pin
	EnableInt( VECT( ICU, IRQ4 ), 1 );				// IRQ4 Interrupt Level is 4
	PORT6.PDR.BIT.B6 = 0;						// Wakuep Signal is Low
	tk_dly_tsk( 5 );						// 5ms Wait
	PORT6.PDR.BIT.B6 = 1;						// Wakuep Signal is High
	while( 1 )  {
		tm_putstring("Gesture Mode.\n\r");			// Gesture Mode
		while( 1 )  {
			if( tk_slp_tsk( 1000 * 10 ) == E_TMOUT )	// Wait FT5602 /INT Signal
				break;					// Mode Change
			tk_swri_dev( dd, 0x38, "\x01",  1, &asize );	// Select GEST_ID
			tk_srea_dev( dd, 0x38, &num, 1, &asize );	// Read Gesture ID
			switch( num )  {				// Gesture Check
			case 0x10: tm_putstring("Move UP");    break;
			case 0x14: tm_putstring("Move Left");  break;
			case 0x18: tm_putstring("Move Down");  break;
			case 0x1C: tm_putstring("Move Right"); break;
			case 0x48: tm_putstring("Zoom In");    break;
			case 0x49: tm_putstring("Zoom Out");   break;
			}
			if( num )
				tm_putstring("\n\r");			// CR,LF
		}
		tm_putstring("Touch Point Mode.\n\r");			// Touch Point Mode
		while( 1 )  {
			if( tk_slp_tsk( 1000 * 10 ) == E_TMOUT )	// Wait FT5602 /INT Signal
				break;					// Mode Change
			tk_swri_dev( dd, 0x38, "\x02",  1, &asize );	// Select TD_STATUS
			tk_srea_dev( dd, 0x38, &num, 1, &asize );	// Read Number Touch Point
			if( num < 1 || num > 5 )			// Check Number Touch Point
				continue;
			for( i=0 ; i<num ; i++ )  {
				buf[0] = i * 6 + 3;			// Make Register Address
				tk_swri_dev( dd, 0x38, buf, 1, &asize );// Select TOUCHn_XH
				tk_srea_dev( dd, 0x38, buf, 4, &asize );// Read Touch Point
				switch( buf[0] & 0xC0 )  {		// Event Check
				case 0x00: tm_putstring("Put Down "); break;
				case 0x40: tm_putstring("Put Up   "); break;
				case 0x80: tm_putstring("Contact  "); break;
				}
				x = ( ( buf[0] & 0x0F ) << 8 ) + buf[1];// Make X Position
				y = ( ( buf[2] & 0x0F ) << 8 ) + buf[3];// Make Y Position
				tm_printf(" #%d x =%4d, y =%4d\n\r", i+1, x, y);
			}
		}
	}
}

EXPORT void irq4_hdr(UINT intno)
{
	tk_wup_tsk( ObjID[TOUCH_TSK] );				// Wakeup Touch Task
}