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

EXPORT void sensor_tsk(INT stacd, void *exinf);

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
ID objid;

	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task =  sensor_tsk;				// Set Task Function Address
	strcpy( t_ctsk.dsname, "sensor" );			// Set Object Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Sensor Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) != E_OK )			// Start Sensor Task
		goto ERROR;
	while( 1 )  tk_slp_tsk(TMO_FEVR);			// Task Waiting
ERROR:
	return 0;
}

EXPORT void sensor_tsk(INT stacd, void *exinf)
{
ID dd;
UB buf[2];
SZ asize;
INT data;
	dd = tk_opn_dev( "siicl", TD_UPDATE );			// Open Simple IIC Driver
	tk_swri_dev( dd, 0x88, "\x0F\x28", 2, &asize );		// Initialize ISL29034
	tk_swri_dev( dd, 0x88, "\x00\xC0", 2, &asize );		// Set Measures IR Mode
	while( 1 )  {
		tk_swri_dev( dd, 0x88, "\x02",  1, &asize );	// Select Data(LSB)
		tk_srea_dev( dd, 0x88, &buf[0], 1, &asize );	// Read D7-D0
		tk_swri_dev( dd, 0x88, "\x03",  1, &asize );	// Select Data(MSB)
		tk_srea_dev( dd, 0x88, &buf[1], 1, &asize );	// Read D15-D8
		data = ( buf[1] << 8 ) + buf[0];		// Make Light Sensor Value
		tm_printf("%d\n",data);				// Output Console
		tk_dly_tsk( 500 );				// Wait 500ms
	}
}