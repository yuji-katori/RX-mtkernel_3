/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	qspi_driver.c
 */

#include <string.h>
#include <tk/tkernel.h>
#include <dev_qspi.h>
#include "qspi_config.h"
#include "iodefine.h"

LOCAL UW wtaddr, rdaddr;

LOCAL void QSPI_EndCheck(void)
{
	QSPI.SPCR.BIT.SPE = 1;					// Operation Enable
	while( ! QSPI.SPSR.BIT.SPSSLF )  ;			// Wait Detect QSSL Negation
	QSPI.SPCR.BIT.SPE = 0;					// Operation Disable
	while( QSPI.SPCR.BIT.SPE )  ;				// Wait Operation Disable
	QSPI.SPSR.BIT.SPSSLF = 0;				// Clear QSSL Negation Detect Flag
}

EXPORT TMO QSPI_WtCheck(void)
{
	QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
	QSPI.SPBMUL0 = 1;					// 1 Count
	QSPI.SPCMD1.WORD = 0xE013;				// Single, Byte, Read Operation
	QSPI.SPBMUL1 = 1;					// 1 Count
	QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
	QSPI.SPDR.BYTE.HH = 0x05;				// Set RDSR Command Code
	QSPI_EndCheck( );					// Operation End Check
	if( QSPI.SPDR.WORD.H & 0x01 )				// WIP of SR Check
		return  1;					// Write Command Not Complete
	QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
//	QSPI.SPBMUL0 = 1;					// 1 Count
	QSPI.SPSCR.BYTE = 0x00;					// CMD0
	QSPI.SPDR.BYTE.HH = 0x04;				// Set WRDI Command Code
	QSPI_EndCheck( );					// Operation End Check
	if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read
	return TMO_FEVR;					// Write Command Complete
}

EXPORT void QSPI_Read(T_DEVREQ *devreq)
{
UB CMD = 0;
INT i;
	switch( devreq->start )  {
	case DN_RDCR:							// (R) Read Configuration Register
		CMD += 0x10;						// Set RDCR Command Code Offset
	case DN_RDSR:							// (R) Read Status Register
		CMD += 0x05;						// Set RDSR Command Code Offset
		if( devreq->size != 1 || devreq->buf == NULL )  {	// Parameter Check
			devreq->error = E_PAR;				// Set Error Code
			break;
		}
		QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPCMD1.WORD = 0xE013;				// Single, Byte, Read Operation
		QSPI.SPBMUL1 = 1;					// 1 Count
		QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
		QSPI.SPDR.BYTE.HH = CMD;				// Set RDCR or RDSR Command Code
		QSPI_EndCheck( );					// Operation End Check
		((UB*)devreq->buf)[0] = QSPI.SPDR.WORD.H;		// Set SR or CR Value
		devreq->asize = 1;					// Set Actual Size
		devreq->error = E_OK;					// set Error Code
		break;
	case DN_QREAD:							// (R) Quad Read
		CMD += 0x30;						// Set QREAD Command Code Offset
	case DN_DREAD:							// (R) Dual Read
		CMD += 0x30;						// Set DREAD Command Code Offset
	case DN_FREAD:							// (R) Fast Read
		CMD += 0x0B;						// Set FREAD Command Code Offset
		if( devreq->size > 0x400000UL || devreq->buf == NULL )  {
			devreq->error = E_PAR;				// Set Error Code
			break;
		}
		QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPCMD1.WORD = 0xE283;				// Single, Long, Write Operation
		QSPI.SPBMUL1 = 1;					// 1 Count
		QSPI.SPCMD2.WORD = 0xE013 + ( CMD / 0x30 << 5 );	// S or D or Q, Byte, Read Operation
		QSPI.SPBMUL2 = devreq->size;				// Set Byte Count
		QSPI.SPSCR.BYTE = 0x02;					// CMD0 --> CMD1 --> CMD2
		QSPI.SPDR.BYTE.HH = CMD;				// Set RDCR or RDSR Command Code
		QSPI.SPDR.LONG = rdaddr << 8;				// Set Address + Dummy
		QSPI.SPCR.BIT.SPE = 1;					// Operation Enable
		for( i=0 ; i<5 ; i++ )  {				// Dummy Read Loop
			while( ! QSPI.SPBDCR.BIT.RXBC )  ;		// Exist Receive Data
			if( QSPI.SPDR.BYTE.HH )  ;			// Dummy Read
		}
		for( i=0 ; i<devreq->size ; i++ )  {			// Receive Data Loop
			while( ! QSPI.SPBDCR.BIT.RXBC )  ;		// Exist Receive Data
			((UB*)devreq->buf)[i] = QSPI.SPDR.BYTE.HH;	// Receive Data Read
		}
		while( ! QSPI.SPSR.BIT.SPSSLF )  ;			// Wait Detect QSSL Negation
		QSPI.SPCR.BIT.SPE = 0;					// Operation Disable
		while( QSPI.SPCR.BIT.SPE )  ;				// Wait Operation Disable
		QSPI.SPSR.BIT.SPSSLF = 0;				// Clear QSSL Negation Detect Flag
		while( QSPI.SPBDCR.BIT.RXBC )				// Exist Receive Data
			if( QSPI.SPDR.BYTE.HH )  ;			// Dummy Read
		rdaddr = ( rdaddr + devreq->size ) & 0x3FFFFF;		// Write Address Adjust
		devreq->asize = devreq->size;				// Set Actual Size
		devreq->error = E_OK;					// Set Normal End
		break;
	default:
		devreq->error = E_NOSPT;				// Set Error Code
	}
}

EXPORT TMO QSPI_Write(T_DEVREQ *devreq)
{
UB SR, CR, CMD = 0;
INT i;
	switch( devreq->start )  {
	case DN_WTADDR:							// (W) Set Write Address
	case DN_RDADDR:							// (W) Set Read Address
		if( devreq->size != 4 || devreq->buf == NULL )  {	// Parameter Check
			devreq->error = E_PAR;				// Set Error Code
			break;
		}
		if( devreq->start == DN_WTADDR )			// DN_WTADDR Command ?
			wtaddr = ((UW*)devreq->buf)[0] & 0x3FFFFF;	// Modify Write Address
		else
			rdaddr = ((UW*)devreq->buf)[0] & 0x3FFFFF;	// Modify Read Address
		devreq->asize = 4;					// Set Actual Size
		devreq->error = E_OK;					// Set Normal End
		break;
	case DN_WRSR:							// (W) Write Status Register
		if( devreq->size != 2 || devreq->buf == NULL )  {	// Parameter Check
			devreq->error = E_PAR;				// Set Error Code
			break;
		}
		QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPCMD1.WORD = 0xE013;				// Single, Byte, Read Operation
		QSPI.SPBMUL1 = 1;					// 1 Count
		QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
		QSPI.SPDR.BYTE.HH = 0x05;				// Set RDSR Command Code
		QSPI_EndCheck( );					// Operation End Check
		SR = QSPI.SPDR.WORD.H & 0xC3;				// Read SR Value
//		QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
//		QSPI.SPBMUL0 = 1;					// 1 Count
//		QSPI.SPCMD1.WORD = 0xE013;				// Single, Byte, Read Operation
//		QSPI.SPBMUL1 = 1;					// 1 Count
//		QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
		QSPI.SPDR.BYTE.HH = 0x15;				// Set RDCR Command Code
		QSPI_EndCheck( );					// Operation End Check
		CR = QSPI.SPDR.WORD.H & 0xF7;				// Read CR Value
		SR |= ((UB*)devreq->buf)[0] & 0xBC;			// Set SRWD, BP3-BP0
		CR |= ((UB*)devreq->buf)[1] & 0x08;			// Set TB
		QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPSCR.BYTE = 0x00;					// CMD0
		QSPI.SPDR.BYTE.HH = 0x06;				// Set WREN Command Code
		QSPI_EndCheck( );					// Operation End Check
//		if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read
//		QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 3;					// 1 Count
//		QSPI.SPSCR.BYTE = 0x00;					// CMD0
		QSPI.SPDR.BYTE.HH = 0x01;				// Set WRSR Command Code
		QSPI.SPDR.BYTE.HH = SR;					// Set Write SR Value
		QSPI.SPDR.BYTE.HH = CR;					// Set Write CR Value
		QSPI_EndCheck( );					// Operation End Check
		if( QSPI.SPDR.LONG )  ;					// Read Receive Buffer
		devreq->asize = 2;					// Set Actual Size
		devreq->error = E_OK;					// Set Normal End
		return 1;						// Wait Write in Progress End
	case DN_BE:							// (W) Block Erase (64K Byte)
		CMD += 0x86;						// Set BE Commmand Offset
	case DN_BE32K:							// (W) Block Erase (32K Byte)
		CMD += 0x32;						// Set BE32K Commmand Offset
	case DN_SE:							// (W) Section Erase (4K Byte)
		CMD += 0x20;						// Set SE Commmand Offset
		if( devreq->size != 4 || devreq->buf == NULL )  {	// Parameter Check
			devreq->error = E_PAR;				// Set Error Code
			break;
		}
		QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPSCR.BYTE = 0x00;					// CMD0
		QSPI.SPDR.BYTE.HH = 0x06;				// Set WREN Command Code
		QSPI_EndCheck( );					// Operation End Check
		if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read
		QSPI.SPCMD0.WORD = 0xE203;				// Single, Long, Write Operation
//		QSPI.SPBMUL0 = 1;					// 1 Count
//		QSPI.SPSCR.BYTE = 0x00;					// CMD0
		QSPI.SPDR.LONG = ( CMD << 24 ) + ( ((UW*)devreq->buf)[0] & 0x3FFFFF );	// Set CE or BE32K or BE Command Code
		QSPI_EndCheck( );					// Operation End Check
		if( QSPI.SPDR.LONG )  ;					// Dummy Read
		devreq->asize = 4;					// Set Actual Size
		devreq->error = E_OK;					// Set Normal End
		return 1;						// Wait Write in Progress End
	case DN_CE:							// (W) Chip Erase
		QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPSCR.BYTE = 0x00;					// CMD0
		QSPI.SPDR.BYTE.HH = 0x06;				// Set WREN Command Code
		QSPI_EndCheck( );					// Operation End Check
		if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read
//		QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
//		QSPI.SPBMUL0 = 1;					// 1 Count
//		QSPI.SPSCR.BYTE = 0x00;					// CMD0
		QSPI.SPDR.BYTE.HH = 0x60;				// Set CE Command Code
		QSPI_EndCheck( );					// Operation End Check
		if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read
		devreq->asize = 0;					// Set Actual Size
		devreq->error = E_OK;					// Set Normal End
		return 1;						// Wait Write in Progress End
	case DN_PP:							// (W) Page Program
		if( ( wtaddr & 0xFF ) + devreq->size > 0x100UL || devreq->buf == NULL )  {
			devreq->error = E_PAR;				// Set Error Code
			break;
		}
		QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPSCR.BYTE = 0x00;					// CMD0
		QSPI.SPDR.BYTE.HH = 0x06;				// Set WREN Command Code
		QSPI_EndCheck( );					// Operation End Check
		if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read
		QSPI.SPCMD0.WORD = 0xE283;				// Single, Long, Write Operation
		QSPI.SPBMUL0 = 1;					// 1 Count
		QSPI.SPCMD1.WORD = 0xE003;				// Single, Byte, Read Operation
		QSPI.SPBMUL1 = devreq->size;				// Set Byte Count
		QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
		QSPI.SPDR.LONG = ( 0x02UL << 24 ) + wtaddr;		// Set PP Command Code & Write Address
		QSPI.SPCR.BIT.SPE = 1;					// Operation Enable
		for( i=0 ; i<devreq->size ; i++ )  {			// Send Data Loop
			while( QSPI.SPBDCR.BIT.TXBC == 32 )  ;		// Wait Send Buffer Empty
			QSPI.SPDR.BYTE.HH = ((UB*)devreq->buf)[i];	// Set Write Data
			if( QSPI.SPBDCR.BIT.RXBC )			// Exist Receive Data
				if( QSPI.SPDR.BYTE.HH )  ;		// Dummy Read
		}
		while( ! QSPI.SPSR.BIT.SPSSLF )  ;			// Wait Detect QSSL Negation
		QSPI.SPCR.BIT.SPE = 0;					// Operation Disable
		while( QSPI.SPCR.BIT.SPE )  ;				// Wait Operation Disable
		QSPI.SPSR.BIT.SPSSLF = 0;				// Clear QSSL Negation Detect Flag
		while( QSPI.SPBDCR.BIT.RXBC )				// Exist Receive Data
			if( QSPI.SPDR.BYTE.HH )  ;			// Dummy Read
		wtaddr = ( wtaddr + devreq->size ) & 0x3FFFFF;		// Write Address Adjust
		devreq->asize = devreq->size;				// Set Actual Size
		devreq->error = E_OK;					// Set Normal End
		return 1;						// Wait Write in Progress End
	default:
		devreq->error = E_NOSPT;				// Set Error Code
	}
	return TMO_FEVR;
}

EXPORT void QSPI_Init(void)
{
UB SR, CR;
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( QSPI ) )  {					// QSPI is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return;						// QSPI is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( QSPI ) = 0;					// Enable QSPI
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
	MPC.PD7PFS.BYTE = 0x1B;					// PD7 is QMI-B/QIO1-B
	MPC.PD6PFS.BYTE = 0x1B;					// PD6 is QMO-B/QIO0-B
	MPC.PD5PFS.BYTE = 0x1B;					// PD5 is QSPCLK-B
	MPC.PD4PFS.BYTE = 0x1B;					// PD4 is QSSL-B
	MPC.PD3PFS.BYTE = 0x1B;					// PD3 is QIO3-B
	MPC.PD2PFS.BYTE = 0x1B;					// PD2 is QIO2-B
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	PORTD.PMR.BYTE |= 0xFC;					// PD7-PD2 is Peripheral Pin
	tk_ena_dsp( );						// Dispatch Enable
	QSPI.SPDCR.BYTE = 0x80;					// Enable Dummy Send
	QSPI.SPBR.BYTE = 1;					// 30MHz
	QSPI.SPCR.BYTE = 0x08;					// Set Master Mode

	QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
	QSPI.SPBMUL0 = 1;					// 1 Count
	QSPI.SPCMD1.WORD = 0xE013;				// Single, Byte, Read Operation
	QSPI.SPBMUL1 = 1;					// 1 Count
	QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
	QSPI.SPDR.BYTE.HH = 0x15;				// Set RDCR Command
	QSPI_EndCheck( );					// Operation End Check
	CR = QSPI.SPDR.WORD.H;					// Read CR Value
//	QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
//	QSPI.SPBMUL0 = 1;					// 1 Count
//	QSPI.SPCMD1.WORD = 0xE013;				// Single, Byte, Read Operation
//	QSPI.SPBMUL1 = 1;					// 1 Count
//	QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
	QSPI.SPDR.BYTE.HH = 0x05;				// Set RDSR Command
	QSPI_EndCheck( );					// Operation End Check
	SR = QSPI.SPDR.WORD.H;					// Read SR Value
	if( SR & 0x40 )						// Quad Enable ?
		return;						// Quad Enable

	QSPI.SPCMD0.WORD = 0xE003;				// Single, Byte, Write Operation
//	QSPI.SPBMUL0 = 1;					// 1 Count
	QSPI.SPSCR.BYTE = 0x00;					// CMD0
	QSPI.SPDR.BYTE.HH = 0x06;				// Set WREN Command Code
	QSPI_EndCheck( );					// Operation End Check
	if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read

	QSPI.SPCMD0.WORD = 0xE083;				// Single, Byte, Write Operation
//	QSPI.SPBMUL0 = 1;					// 1 Count
	QSPI.SPCMD1.WORD = 0xE003;				// Single, Byte, Write Operation
	QSPI.SPBMUL1 = 2;					// 1 Count
	QSPI.SPSCR.BYTE = 0x01;					// CMD0 --> CMD1
	QSPI.SPDR.BYTE.HH = 0x01;				// Set WRSR Command
	QSPI.SPDR.BYTE.HH = SR |= 0x40;				// Set SR Value (QE=1)
	QSPI.SPDR.BYTE.HH = CR;					// Set CR Value
	QSPI_EndCheck( );					// Operation End Check
	if( QSPI.SPDR.BYTE.HH )  ;				// Dummy Read
	if( QSPI.SPDR.WORD.H )  ;				// Dummy Read
}

EXPORT PRI QSPI_GetTaskPri(void)
{
	return QSPI_CFG_TASK_PRIORITY;				// Return Task Priority
}