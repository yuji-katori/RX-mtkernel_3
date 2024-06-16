/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usermain_i2c.c (usermain)
 *	User Main
 */

#include <string.h>
#include <tk/tkernel.h>
#include "dev_sci.h"

#define	LEN	5

EXPORT void echo_tsk(INT stacd, void *exinf)
{
RsMode rsm = { 0, 3, 0, 115200 };						// 8bit,Non Parity,1Stop bit
ID dd, reqid;
VB buf[2][LEN];
SZ asize;
ER ioer;
	if( ( dd = tk_opn_dev( "scia", TD_UPDATE ) ) <= E_OK )			// Open SCI Driver
		goto Err2;
	if( tk_swri_dev( dd, DN_RSMODE, &rsm, sizeof(rsm), &asize ) < E_OK )	// Set Communication Mode
		goto Err1;
	while( 1 )  {
		reqid = tk_rea_dev( dd, 0, buf[0], LEN, TMO_FEVR );		// Receive Request
		while( reqid != tk_wai_dev( dd, 0, &asize, &ioer, TMO_FEVR ) )	// Wait Request End
			;
		strncpy( buf[1], buf[0], LEN );					// Copy Receive Data
		tk_wri_dev( dd, 0, buf[1], LEN, TMO_FEVR );			// Echo Back(Send Request)
	}
Err1:
	tk_cls_dev( dd, 0 );							// Close IIC or SIIC Driver
Err2:
	tk_ext_tsk( );								// Exit
}

EXPORT INT usermain( void )
{
ID objid;
T_CTSK t_ctsk;

	sciDrvEntry( );						// Entry SCI Driver
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack Size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task = echo_tsk;					// Set Task Start Address
	strcpy( t_ctsk.dsname, "echo" );			// Set Debugger Support Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Echo Back Task
		goto Err;
	if( tk_sta_tsk( objid, 0 ) < E_OK )			// Start Echo Back Task
		goto Err;
	tk_slp_tsk( TMO_FEVR );					// Sleep
Err:
	while( 1 )  ;
}