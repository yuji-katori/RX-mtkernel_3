/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usermain_qspi.c (usermain)
 *	User Main
 */

#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "dev_qspi.h"

UB buf[2][256];

EXPORT void sf_tsk(INT stacd, void *exinf)
{
ID dd;
SZ asize;
UW addr = 0;
INT i, j;
	for( i=0 ; i<256 ; i++ )						// Make Write Data
		buf[0][i] = i;
	if( ( dd = tk_opn_dev( QSPISF_DEVNM, TD_UPDATE ) ) <= E_OK )		// Open QSPI Driver
		goto Err;
	tk_swri_dev( dd, DN_WTADDR, &addr, sizeof(UW), &asize );		// Set Write Address
	tk_swri_dev( dd, DN_RDADDR, &addr, sizeof(UW), &asize );		// Set Read Address
	tm_putstring(" Read Data\n\r");
	tk_srea_dev( dd, DN_FREAD, buf[1], sizeof(buf[1]), &asize );		// First Read
	for( i=0 ; i<16 ; i++ )  {						// Make Write Data
		for( j=0 ; j<16 ; j++ )
			tm_printf(" %2X", buf[1][i*16+j]);			// Output Dump Code
		tm_putstring("\n\r");						// Output CR,LF
	}
			
	tm_putstring(" Write Data\n\r");
	tk_swri_dev( dd, DN_PP, buf[0], sizeof(buf[0]), &asize );				// Write Data
	tk_swri_dev( dd, DN_WTADDR, &addr, sizeof(UW), &asize );		// Set Write Address
	tk_swri_dev( dd, DN_RDADDR, &addr, sizeof(UW), &asize );		// Set Read Address
	tm_putstring(" Read Data\n\r");
	tk_srea_dev( dd, DN_DREAD, buf[1], sizeof(buf[1]), &asize );		// Dual Read
	for( i=0 ; i<16 ; i++ )  {						// Make Write Data
		for( j=0 ; j<16 ; j++ )
			tm_printf(" %02X", buf[1][i*16+j]);			// Output Dump Code
		tm_putstring("\n\r");						// Output CR,LF
	}
	tm_putstring(" Section Erase\n\r");
	tk_swri_dev( dd, DN_SE, &addr, sizeof(UW), &asize );			// Section Erase
	tk_swri_dev( dd, DN_RDADDR, &addr, sizeof(UW), &asize );		// Set Read Address
	tm_putstring(" Read Data\n\r");
	tk_srea_dev( dd, DN_QREAD, buf[1], sizeof(buf[1]), &asize );		// Quad Read
	for( i=0 ; i<16 ; i++ )  {						// Make Write Data
		for( j=0 ; j<16 ; j++ )
			tm_printf(" %2X", buf[1][i*16+j]);			// Output Dump Code
		tm_putstring("\n\r");						// Output CR,LF
	}
Err:
	tk_ext_tsk( );								// Exit
}

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
ID objid;

	qspiDrvEntry( );					// Entry QSPI Driver
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task =  sf_tsk;					// Set Task Function Address
	strcpy( t_ctsk.dsname, "qspitsk" );			// Set Object Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Serial Flash Memory Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) != E_OK )			// Start Serial Flash Memory Task
		goto ERROR;
	while( 1 )  tk_slp_tsk(TMO_FEVR);			// Task Waiting
ERROR:
	return 0;
}