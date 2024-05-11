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

#ifdef AP_RX72N
#define	BSIZE	2
#else
#define	BSIZE	1
#endif

UB buf[128*BSIZE];

EXPORT void eeprom_tsk(INT stacd, void *exinf)
{
ID dd;
SZ asize;	
INT i, j;

	if( ( dd = tk_opn_dev( "siica", TD_UPDATE ) ) <= E_OK )	// Open IIC or SIIC Driver
		goto Err;
	tk_srea_dev( dd, 0x10, buf, 128, &asize );	// Read Data (Mac Address)
	tk_swri_dev( dd, 0x50, "\x00", 1, &asize );		// Set Random Read Address
	tk_srea_dev( dd, 0x50, buf, 128*BSIZE, &asize );	// Read Data (Mac Address)
	tm_printf(" %02X:%02X:%02X:%02X:%02X:%02X\n",buf[0],buf[1],buf[2],buf[3],buf[4],buf[5]);
	for( i=0 ; i<8*BSIZE ; i++ )  {
		for( j=0 ; j<16 ; j++ )
			tm_printf(" %02X",buf[(i<<4)+j]);
		tm_printf("\n");
	}
	for( i=16 ; i<128*BSIZE ; i++ )
		if( buf[i] != 0xFF )
			break;
	if( i == 128*BSIZE )
		for( i=16 ; i<128*BSIZE ; i++ )
			buf[i] = i;
	else
		for( i=16 ; i<128*BSIZE ; i++ )
			buf[i] ++ ;
	for( i=16 ; i<64*BSIZE ; i++ )  {
		buf[i-1] = i;
		tk_swri_dev( dd, 0x50, &buf[i-1], 2, &asize );			// Byte Write
		while( tk_swri_dev( dd, 0x50, "\x00", 0, &asize ) != E_OK )	// Acknowledge Polling
			__nop( );
	}
	for( i=64*BSIZE ; i<128*BSIZE ; i+=8*BSIZE )  {
		buf[i-1] = i;
		tk_swri_dev( dd, 0x50, &buf[i-1], 1+8*BSIZE, &asize );		// Page Write
		while( tk_swri_dev( dd, 0x50, "\x00", 0, &asize ) != E_OK )	// Acknowledge Polling
			;
	}
	tm_printf("\n");
	tk_swri_dev( dd, 0x50, "\x00", 1, &asize );		// Set Random Read Address
	tk_srea_dev( dd, 0x50, buf, 128*BSIZE, &asize );	// Read Data (Mac Address)
	for( i=0 ; i<8*BSIZE ; i++ )  {
		for( j=0 ; j<16 ; j++ )
			tm_printf(" %02X",buf[(i<<4)+j]);
		tm_printf("\n");
	}
	tk_cls_dev( dd, 0 );					// Close IIC or SIIC Driver
Err:
	tk_ext_tsk( );						// Exit
}

EXPORT INT usermain( void )
{
ID objid;
T_CTSK t_ctsk;

	iicDrvEntry( );						// Entry I2C Driver
	siicDrvEntry( );					// Entry Simple I2C Driver
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack Size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task =  eeprom_tsk;				// Set Task Start Address
	strcpy( t_ctsk.dsname, "eeprom" );			// Set Debugger Support Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Gesture Task
		goto Err;
	if( tk_sta_tsk( objid, 0 ) < E_OK )			// Start Gesture Task
		goto Err;
	tk_slp_tsk( TMO_FEVR );					// Sleep
Err:
	while( 1 )  ;
}