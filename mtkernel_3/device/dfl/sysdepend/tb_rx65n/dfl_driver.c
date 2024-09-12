/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dfl_driver.c
 */

#include <string.h>
#include <tk/tkernel.h>
#include <dev_dfl.h>
#include "dfl_config.h"
#include "iodefine.h"

#define	MINIMUMFCLK	(20)			// 20MHz
#define	FLASHSIZE	(32*1024)		// 32K Byte
#define	tDP4		(2)			//   1.7ms * 1.1
#define	tDP64		(11)			//  10  ms * 1.1
#define	tDP128		(17)			//  15  ms * 1.1
#define	tDP256		(31)			//  28  ms * 1.1
#define	tDBC4		(1)			//  30  us * 1.1 (Not Used)
#define	tDBC64		(1)			// 100  us * 1.1
#define	tFD		(1)			//  20  us * 1.1
#define	FACI		(*(volatile union { unsigned short WORD;unsigned char BYTE; }*)0x7E0000)

LOCAL ID ObjID;								// Task ID

LOCAL void fcu_frdyi_hdr(UINT dintno)
{
	tk_wup_tsk( ObjID );						// WakeUp Waiting Task
}

LOCAL void forced_stop_command(void)
{
	FACI.BYTE = 0xB3;						// Forced Stop Command
	if( tk_slp_tsk( tFD ) == E_OK )					// Check FRDY Flag
		while( FLASH.FASTAT.BIT.CMDLK )				// Check Command Lock
			FACI.BYTE = 0x50;				// Status Clear
}

EXPORT ER DFL_Open(void)
{
UW clock;
T_DINT t_dint;
	clock = ICLK << SYSTEM.SCKCR.BIT.ICK >> SYSTEM.SCKCR.BIT.FCK;	// Analize FCLK
	clock = clock / 1000000 + ( clock % 1000000 ? 1 : 0 );		// MHz Convert
	if( clock < MINIMUMFCLK )					// Minimum Check
		return E_IO;
	FLASH.FPCKAR.WORD = 0xAA00 + clock;				// Set FPCKAR
	FLASH.EEPFCLK = clock;						// Set EEPFCLK
	while( FLASH.FASTAT.BIT.CMDLK )					// Check Command Lock
		FACI.BYTE = 0x50;					// Status Clear
	t_dint.intatr = TA_HLNG;					// Set Handler Attribute
#ifdef CLANGSPEC
	t_dint.inthdr = fcu_frdyi_hdr;					// Set Handler Address
#else
	t_dint.inthdr = (FP)fcu_frdyi_hdr;				// Set Handler Address
#endif
	tk_def_int( VECT( FCU, FRDYI ), &t_dint );			// Define Interrupt Handler
	EnableInt( VECT( FCU, FRDYI ), DFL_CFG_INT_PRIORITY );		// Enable FCU FRDYI Interrupt
	FLASH.FRDYIE.BYTE = 0x01;					// Enable FRDYI
	return E_OK;
}

EXPORT ER DFL_Close(void)
{
T_DINT t_dint;
	FLASH.FRDYIE.BYTE = 0x00;					// Disable FRDYI
	DisableInt( VECT( FCU, FRDYI ) );				// Disable FCU FRDYI Interrupt
	t_dint.intatr = TA_HLNG;					// Set Handler Attribute
	t_dint.inthdr = NULL;						// Set Handler Address
	tk_def_int( VECT( FCU, FRDYI ), &t_dint );			// Undefine Interrupt Handler
	FLASH.EEPFCLK = 0x3C;						// Reset EEPFCLK
	FLASH.FPCKAR.WORD = 0xAA00 + 60;				// Reset FPCKAR
	return E_OK;
}

EXPORT void DFL_Read(T_DEVREQ *devreq)
{
UW *src, *dst;
INT i;
	src = (UW *)(0x100000 + devreq->start);				// Make Source Address
	dst = devreq->buf;						// Make Destination Address
	for( i=0 ; i<devreq->size ; i++ )				// Read Flash Memory
		*dst++ = *src++;
	devreq->asize = devreq->size;					// Set Actual Size
	devreq->error = E_OK;						// Set Error Code
}

EXPORT void DFL_Write(T_DEVREQ *devreq)
{
UW *src, *dst, data;
INT i;
	ObjID = tk_get_tid( );						// Get Request Task ID
	if( tk_slp_tsk( TMO_POL ) == E_OK )  {				// Check Wakeup Count
		tk_wup_tsk( ObjID );					// Release Wakeup Count
		devreq->error = E_OBJ;					// Set Object Status Error
		return;
	}
	src = devreq->buf;						// Make Source Address
	dst = (UW *)devreq->start;					// Make Destination Address
	devreq->asize = 0;						// Initialize Actual Size
	FLASH.FWEPROR.BYTE = 0x01;					// Enable P/E
	FLASH.FENTRYR.WORD = 0xAA80;					// Migrate P/E Mode
	while( FLASH.FENTRYR.WORD != 0x0080 )  ;			// Check Write Value
	for( i=0 ; i<devreq->size ; i++ )  {				// Write Flash Memory
		FLASH.FSADDR.LONG = 0x100000 + (UW)dst++;		// Set Start Address
		FACI.BYTE = 0xE8;					// Set Program Command
		FACI.BYTE = 0x02;					// Set Write Count
		data = *src++;						// Read Write Data
		FACI.WORD = data;					// Write Lower Word
		data =__revl( data );					// Word Reverse
		FACI.WORD = data;					// Write Higher Word
		FACI.BYTE = 0xD0;					// Set Execute Command
		if( tk_slp_tsk( tDP4 ) == E_OK )  {			// Check FRDY Flag
			while( FLASH.FASTAT.BIT.CMDLK )			// Check Command Lock
				FACI.BYTE = 0x50;			// Status Clear
			devreq->asize ++ ;				// Increment Actual Size
		}
		else  {
			forced_stop_command( );				// Forced Stop Command
			break;						// I/O Error
		}
	}
	FLASH.FENTRYR.WORD = 0xAA00;					// Migrate Normal Mode
	while( FLASH.FENTRYR.WORD != 0x0000 )  ;			// Check Write Value
	FLASH.FWEPROR.BYTE = 0x10;					// Disable P/E
	devreq->error = i == devreq->size ? E_OK : E_IO;		// Set Error Code
}

EXPORT ER DFL_Blank_Check(UW fsaddr, UW feaddr)
{
ER ercd;
	ObjID = tk_get_tid( );						// Get Request Task ID
	if( tk_slp_tsk( TMO_POL ) == E_OK )  {				// Check Wakeup Count
		tk_wup_tsk( ObjID );					// Release Wakeup Count
		return E_OBJ;						// Set Object Status Error
	}
	FLASH.FWEPROR.BYTE = 0x01;					// Enable P/E
	FLASH.FENTRYR.WORD = 0xAA80;					// Migrate P/E Mode
	while( FLASH.FENTRYR.WORD != 0x0080 )  ;			// Check Write Value
	FLASH.FSADDR.LONG = fsaddr + 0x100000;				// Set Blank Check Start Address
	FLASH.FEADDR.LONG = feaddr + 0x100000;				// Set Blank Check End   Address
//	FLASH.FBCCNT.BYTE = 0x00;					// Additional Mode
	FACI.BYTE = 0x71;						// Blank Check Command
	FACI.BYTE = 0xD0;						// Execute Command
	if( tk_slp_tsk( tDP64 ) == E_OK )  {				// Check FRDY Flag
		while( FLASH.FASTAT.BIT.CMDLK )				// Check Command Lock
			FACI.BYTE = 0x50;				// Status Clear
		ercd = FLASH.FBCSTAT.BIT.BCST ? E_IO : E_OK;		// Not Blank or Blank ?
	}
	else  {
		forced_stop_command( );					// Forced Stop Command
		ercd = E_IO;						// I/O Error
	}
	FLASH.FENTRYR.WORD = 0xAA00;					// Migrate Normal Mode
	while( FLASH.FENTRYR.WORD != 0x0000 )  ;			// Check Write Value
	FLASH.FWEPROR.BYTE = 0x10;					// Disable P/E
	return ercd;
}

EXPORT ER DFL_Block_Erase(UW fsaddr, UW feaddr)
{
ER ercd;
TMO tmout;
	ObjID = tk_get_tid( );						// Get Request Task ID
	if( tk_slp_tsk( TMO_POL ) == E_OK )  {				// Check Wakeup Count
		tk_wup_tsk( ObjID );					// Release Wakeup Count
		return E_OBJ;						// Set Object Status Error
	}
	switch( feaddr - fsaddr )  {					// Erase Length
	case 60:	tmout = tDP64;		break;			//  64 Byte Erase
	case 124:	tmout = tDP128;		break;			// 128 Byte Erase
	case 252:	tmout = tDP256;		break;			// 256 Byte Erase
	}
	FLASH.FWEPROR.BYTE = 0x01;					// Enable P/E
	FLASH.FENTRYR.WORD = 0xAA80;					// Migrate P/E Mode
	while( FLASH.FENTRYR.WORD != 0x0080 )  ;			// Check Write Value
	FLASH.FSADDR.LONG = fsaddr + 0x100000;				// Set Block Erase Start Address
	FLASH.FEADDR.LONG = feaddr + 0x100000;				// Set Block Erase End   Address
	FACI.BYTE = 0x21;						// Erase Command
	FACI.BYTE = 0xD0;						// Execute Command
	if( tk_slp_tsk( tmout ) == E_OK )  {				// Check FRDY Flag
		while( FLASH.FASTAT.BIT.CMDLK )				// Check Command Lock
			FACI.BYTE = 0x50;				// Status Clear
		ercd = E_OK;						// Normal End
	}
	else  {
		forced_stop_command( );					// Forced Stop Command
		ercd = E_IO;						// I/O Error
	}
	FLASH.FENTRYR.WORD = 0xAA00;					// Migrate Normal Mode
	while( FLASH.FENTRYR.WORD != 0x0000 )  ;			// Check Write Value
	FLASH.FWEPROR.BYTE = 0x10;					// Disable P/E
	return ercd;
}

EXPORT size_t DFL_GetFlashSize(void)
{
	return FLASHSIZE;
}
