/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
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
#include "dev_iic.h"
#include "dev_siic.h"

EXPORT void light_tsk(INT stacd, void *exinf)
{
ID dd;
UB buf[2];
SZ asize;	
INT data;

	if( ( dd = tk_opn_dev( "iica", TD_UPDATE ) ) <= E_OK )	// Open IIC or SIIC Driver
		goto Err2;
	if( tk_swri_dev( dd, 0x23, "\x10", 1, &asize ) < E_OK )	// Set Continuously H-Resolution Mode
		goto Err1;
	tk_dly_tsk( 180 );					// Wait 180ms
	while( 1 )  {
		if( tk_srea_dev( dd, 0x23, buf, 2, &asize ) < E_OK )
			continue;				// Read Measurement Result
		data = ( buf[0] << 8 ) + buf[1];		// Make Measurement Result
		tm_printf("%d\n",data);				// Output Console
		tk_dly_tsk( 1000 );				// Wait 500ms
	}
Err1:
	tk_cls_dev( dd, 0 );					// Close IIC or SIIC Driver
Err2:
	tk_ext_tsk( );						// Exit
}

EXPORT void gesture_tsk(INT stacd, void *exinf)
{
ID dd;
UB buf;
UH data;
SZ asize;
INT i;
LOCAL const UB IniReg[][2] = {					// Initialize Data
	{0xEF,0x00},{0x37,0x07},{0x38,0x17},{0x39,0x06},{0x42,0x01},{0x46,0x2D},{0x47,0x0F},{0x48,0x3C},{0x49,0x00},{0x4A,0x1E},
	{0x4C,0x20},{0x51,0x10},{0x5E,0x10},{0x60,0x27},{0x80,0x42},{0x81,0x44},{0x82,0x04},{0x8B,0x01},{0x90,0x06},{0x95,0x0A},
	{0x96,0x0C},{0x97,0x05},{0x9A,0x14},{0x9C,0x3F},{0xA5,0x19},{0xCC,0x19},{0xCD,0x0B},{0xCE,0x13},{0xCF,0x64},{0xD0,0x21},
	{0xEF,0x01},{0x02,0x0F},{0x03,0x10},{0x04,0x02},{0x25,0x01},{0x27,0x39},{0x28,0x7F},{0x29,0x08},{0x3E,0xFF},{0x5E,0x3D},
	{0x65,0x96},{0x67,0x97},{0x69,0xCD},{0x6A,0x01},{0x6D,0x2C},{0x6E,0x01},{0x72,0x01},{0x73,0x35},{0x77,0x01},{0xEF,0x00},};
LOCAL const VB *msg[] = { "down", "up", "right", "left", "near", "far", "R turn", "L turn", "ByeBye" };
	if( ( dd = tk_opn_dev( "iica", TD_UPDATE ) ) <= E_OK )	// Open IIC or SIIC Driver
		goto Err2;
	tk_dly_tsk( 1 );
	tk_swri_dev( dd, 0x73, "\x00", 1, &asize );		// Gesture Sensor Wakeup
	for( i=0 ; i<20 ; i++ )  {
		tk_swri_dev( dd, 0x73, "\x00", 1, &asize );	// Gesture Sensor Wakeup
		tk_srea_dev( dd, 0x73, &buf, 1, &asize );	// Status Read
		if( buf == 0x20 )				// Wakeup End ?
			break;
	}
	if( i == 20 )
		goto Err1;
	for( i=0 ; i < sizeof(IniReg)/sizeof(IniReg[0]) ; i++ )	// Gesture Sensor Initialize
		tk_swri_dev( dd, 0x73, IniReg[i], 2, &asize );	// Initialize Command
	for( i=0 ; i<2 ; i++ )  {
		tk_swri_dev( dd, 0x73, "\x43", 1, &asize );	// Indicate Register
		tk_srea_dev( dd, 0x73, &buf, 1, &asize );	// Dummy Read
	}
	while( 1 )  {
		tk_dly_tsk( 250 );
		tk_swri_dev( dd, 0x73, "\x43", 1, &asize );	// Indicate Upper Register
		data = 0;					// Data Clear
		tk_srea_dev( dd, 0x73, &data, 2, &asize );	// Read Sensor Value
		if( ! ( data &= 0x1FF ) )			// Find Gesture ?
			continue;
#ifdef __BIG
		data = data / 256 + ( data % 256 << 8 );	// Byte Swap
#endif
		for( i=0 ; data >>= 1 ; i++ )  ;		// Analize Gesture
		tm_printf("%s\n", msg[i]);			// Output Gesture Message
	}
Err1:
	tk_cls_dev( dd, 0 );					// Close IIC Driver
Err2:
	tk_ext_tsk( );						// Exit
}

EXPORT INT usermain( void )
{
ID objid;
T_CTSK t_ctsk;

	iicDrvEntry( );						// Entry IIC Driver
	siicDrvEntry( );					// Entry SIIC Driver
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack Size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task =  gesture_tsk;				// Set Task Start Address
	strcpy( t_ctsk.dsname, "gesture" );			// Set Debugger Support Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Gesture Task
		goto Err;
	if( tk_sta_tsk( objid, 0 ) < E_OK )			// Start Gesture Task
		goto Err;
	t_ctsk.task =  light_tsk;				// Set Task Start Address
	strcpy( t_ctsk.dsname, "light" );			// Set Debugger Support Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Light Task
		goto Err;
	if( tk_sta_tsk( objid, 0 ) < E_OK )			// Start Light Task
		goto Err;
	tk_slp_tsk( TMO_FEVR );					// Sleep
Err:
	while( 1 )  ;
}
