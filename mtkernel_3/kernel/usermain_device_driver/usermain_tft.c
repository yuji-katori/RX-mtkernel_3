/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usermain_tft.c (usermain)
 *	User Main
 */

#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "dev_tft.h"

CONST VB *buf[] = { "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ\n\r",
		    "[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~" };

EXPORT void display_tsk(INT stacd, void *exinf)
{
ID dd;
SZ asize;
INT i;
	if( ( dd = tk_opn_dev( TFT_DEVNM, TD_WRITE ) ) <= E_OK )		// Open TFT Driver
		goto Err;
	while( 1 )  {
		for( i=0 ; i<58 ; i++ )  {
			tk_swri_dev( dd, 0, buf[0]+i, 0, &asize );		// String Output
			tm_printf(" %2d\n", asize);				// Output Actual Size
			tk_dly_tsk( 1000 );					// Wait 1000ms
		}
		for( i=0 ; i<35 ; i++ )  {
			tk_swri_dev( dd, 0, buf[1], 35-i, &asize );		// String Output
			tm_printf(" %2d\n", asize);				// Output Actual Size
			tk_swri_dev( dd, 0, "\n\r", 2, &asize );		// CR, LF Output
			tk_dly_tsk( 1000 );					// Wait 1000ms
		}
	}
Err:
	tk_ext_tsk( );								// Exit
}

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
ID objid;

	tftDrvEntry( );						// Entry TFT Driver
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