/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usermain_dfl.c (usermain)
 *	User Main
 */

#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "dev_dfl.h"

char buf[64];

EXPORT void flash_tsk(INT stacd, void *exinf)
{
ID dd, devid;
SZ asize;	
INT i;

	if( ( dd = tk_opn_dev( DFL_DEVNM, TD_UPDATE | TD_EXCL) ) <= E_OK )	// Open Data Flash Driver
		goto Err;
	devid = tk_ref_dev( DFL_DEVNM, NULL );					// Get Device ID
	for( i=0 ; i<64 ; i++ )							// Make Write Data
		buf[i] = i + 32;
	tm_putstring("Flash Programming at 0x100000-0x10003F\n\r");
	for( i=0 ; i<16 ; i++ )							// Output Write Data
		tm_printf(" %#.2x = %c  %#.2x = %c  %#.2x = %c  %#.2x = %c\n\r",
		buf[i],buf[i], buf[i+16],buf[i+16], buf[i+32],buf[i+32], buf[i+48],buf[i+48] );
	if( tk_evt_dev( devid, TDV_BLANKCHECK64, (void *)0 ) < E_OK )		// Blank Check
		tk_evt_dev( devid, TDV_BLOCKERASE64, (void *)0 );		// 64 Byte Erase
	tk_swri_dev( dd, 0, buf, sizeof(buf)/sizeof(UW), &asize );		// 64 Byte Program
	tm_putstring("Flash Reading at 0x100000-0x10003F\n\r");
	tk_srea_dev( dd, 0, buf, sizeof(buf)/sizeof(UW), &asize );		// 64 Byte Read
	for( i=0 ; i<16 ; i++ )							// Output Write Data
		tm_printf(" %#.2x = %c  %#.2x = %c  %#.2x = %c  %#.2x = %c\n\r",
		buf[i],buf[i], buf[i+16],buf[i+16], buf[i+32],buf[i+32], buf[i+48],buf[i+48] );
	for( i=0 ; i<64 ; i++ )							// Make Write Data
		buf[i] = i + 64;
	tm_putstring("Flash Programming at 0x100040-0x10007B\n\r");
	for( i=0 ; i<16 ; i++ )							// Output Write Data
		tm_printf(" %#.2x = %c  %#.2x = %c  %#.2x = %c  %#.2x = %c\n\r",
		buf[i],buf[i], buf[i+16],buf[i+16], buf[i+32],buf[i+32], buf[i+48],buf[i+48] );
	if( tk_evt_dev( devid, TDV_BLANKCHECK64, (void *)64 ) < E_OK )		// Blank Check
		tk_evt_dev( devid, TDV_BLOCKERASE64, (void *)64 );		// 64 Byte Erase
	for( i=0 ; i<15 ; i++ )							// 4 * 15 Byte Program
		tk_swri_dev( dd, 64+sizeof(UW)*i, &buf[1+sizeof(UW)*i], 1, &asize );
	tm_putstring("Flash Reading at 0x100040-0x10007B\n\r");
	tk_srea_dev( dd, 64, &buf[1], sizeof(buf)/sizeof(UW)-1, &asize );	// 60 Byte Read
	for( i=0 ; i<16 ; i++ )							// Output Write Data
		tm_printf(" %#.2x = %c  %#.2x = %c  %#.2x = %c  %#.2x = %c\n\r",
		buf[i],buf[i], buf[i+16],buf[i+16], buf[i+32],buf[i+32], buf[i+48],buf[i+48] );
	if( tk_evt_dev( devid, TDV_BLANKCHECK4, (void *)124 ) == E_OK )		// Blank Check
		tm_putstring("0x10007C-0x10007F is Blank !\n\r");
	tk_cls_dev( dd, 0 );							// Close Data Flash Driver
Err:
	tk_ext_tsk( );								// Exit
}

EXPORT INT usermain( void )
{
ID objid;
T_CTSK t_ctsk;

	dflDrvEntry( );						// Entry Data Flash Driver
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// Set Task Attribute
	t_ctsk.stksz = 1024;					// Set Task Stack Size
	t_ctsk.itskpri = 10;					// Set Task Priority
	t_ctsk.task =  flash_tsk;				// Set Task Start Address
	strcpy( t_ctsk.dsname, "D flash" );			// Set Debugger Support Name
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// Create Gesture Task
		goto Err;
	if( tk_sta_tsk( objid, 0 ) < E_OK )			// Start Gesture Task
		goto Err;
	tk_slp_tsk( TMO_FEVR );					// Sleep
Err:
	while( 1 )  ;
}