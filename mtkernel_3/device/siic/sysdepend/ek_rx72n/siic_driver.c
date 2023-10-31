/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	siic_driver.c
 */

#include <string.h>
#include <tk/tkernel.h>
#include <dev_siic.h>
#include "siic_config.h"
#include "iodefine.h"

typedef enum { SIIC11, OBJ_KIND_NUM } OBJ_KIND;
SIIC_TBL siic_tbl[OBJ_KIND_NUM];
#if !USE_IMALLOC
LOCAL INT siic_task_stack[OBJ_KIND_NUM][300/sizeof(INT)];
#endif /* USE_IMALLOC */

EXPORT ER SIIC_Read(T_DEVREQ *devreq, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UB)exinf];
volatile struct st_sci0 __evenaccess *SIIC = siic->siic;
UINT flgptn;
ER ercd;
SZ i;
	SIIC->SIMR3.BYTE = 0x51;				// Generates Start Condition
	while( ! SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait STI Interrupt
	SIIC->SIMR3.BIT.IICSTIF = 0;				// Clear IICSTIF
	while( SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait Clear IICSTIF
	SIIC->SIMR3.BYTE = 0x00;				// Clear Start Condistion
	SIIC->TDR = devreq->start | 0x01;			// Set Slave Address
	tk_wai_flg( siic->flgid, TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );	// Wait TXI Interrupt
	if( ! SIIC->SISR.BIT.IICACKR )  {			// Receive ACK
		for( i=0 ; i<devreq->size ; i++ )  {
			if( i == devreq->size - 1 )		// Next Data is Last ?
				SIIC->SIMR2.BIT.IICACKT = 1;	// Set ACK Bit
			SIIC->SCR.BIT.RIE = 1;			// RIE Enable
			SIIC->TDR = 0xFF;			// Dummy Data Write and Wait RXI Interrupt
			tk_wai_flg( siic->flgid, RXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
			((UB*)devreq->buf)[i] = SIIC->RDR;	// Data Read and Wait TXI Interrupt
			tk_wai_flg( siic->flgid, TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		}
		devreq->asize = devreq->size;			// Set Write Size
		ercd = E_OK;					// Set Error Code (E_OK)
	}
	else  {							// Receive NACK
		devreq->asize = 0;				// Set Write Size
		ercd = E_IO;					// Set Error Code (E_IO:Receive NACK)
	}
	SIIC->SIMR3.BYTE = 0x51;				// Generates Stop Condition
	while( ! SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait STI Interrupt
	SIIC->SIMR3.BIT.IICSTIF = 0;				// Clear IICSTIF
	while( SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait Clear IICSTIF
	SIIC->SIMR3.BYTE = 0xF0;				// Transmitend
	return ercd;
}

EXPORT ER SIIC_Write(T_DEVREQ *devreq, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UB)exinf];
volatile struct st_sci0 __evenaccess *SIIC = siic->siic;
UINT flgptn;
ER ercd;
SZ i;
	SIIC->SIMR3.BYTE = 0x51;				// Generates Start Condition
	while( ! SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait STI Interrupt
	SIIC->SIMR3.BIT.IICSTIF = 0;				// Clear IICSTIF
	while( SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait Clear IICSTIF
	SIIC->SIMR3.BYTE = 0x00;				// Clear Start Condistion
	SIIC->TDR = devreq->start & 0xFE;			// Set Slave Address
	tk_wai_flg( siic->flgid, TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );	// Wait TXI Interrupt
	if( ! SIIC->SISR.BIT.IICACKR )  {			// Receive ACK
		for( i=0 ; i<devreq->size ; i++ )  {
			SIIC->TDR = ((UB*)devreq->buf)[i];	// Data Write and Wait TXI Interrupt
			tk_wai_flg( siic->flgid, TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		}
		devreq->asize = devreq->size;			// Set Write Size
		ercd = E_OK;					// Set Error Code (E_OK)
	}
	else  {							// Receive NACK
		devreq->asize = 0;				// Set Write Size
		ercd = E_IO;					// Set Error Code (E_IO:Receive NACK)
	}
	SIIC->SIMR3.BYTE = 0x51;				// Generates Stop Condition
	while( ! SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait STI Interrupt
	SIIC->SIMR3.BIT.IICSTIF = 0;				// Clear IICSTIF
	while( SIIC->SIMR3.BIT.IICSTIF )  ;			// Wait Clear IICSTIF
	SIIC->SIMR3.BYTE = 0xF0;				// Transmitend
	return ercd;
}

LOCAL void SIIC_RXI_Handler(UB ch)
{
SIIC_TBL *siic = &siic_tbl[ch];
	SCI11.SCR.BIT.RIE = 0;					// RIE Disable
	tk_set_flg( siic->flgid, RXI_INT );			// Set RXI Event
}

LOCAL void SIIC_TXI_Handler(UB ch)
{
SIIC_TBL *siic = &siic_tbl[ch];
	tk_set_flg( siic->flgid, TXI_INT );			// Set TXI Event
}

LOCAL void sci11_rxi11_hdr(UINT dintno)
{
	SIIC_RXI_Handler( SIIC11 );				// Call SIIC_RXI_Handler
}

LOCAL void sci11_txi11_hdr(UINT dintno)
{
	SIIC_TXI_Handler( SIIC11 );				// Call SIIC_TXI_Handler
}

EXPORT ER siicDrvEntry(void)
{
ID objid;
VB *DrvName;
union { T_CTSK t_ctsk; T_CFLG t_cflg; T_DDEV t_ddev; T_DINT t_dint; } u;
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI11 ) )  {				// SCI11 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI11 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI11 ) = 0;					// Enable SCI11
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI11.SIMR3.BYTE = 0xF0;				// SSCL,SSDA to High Impedance
	SCI11.SCMR.BIT.SDIR = 1;				// MSB First Transmit
	SCI11.BRR = PCLKA / 32.0F / 1000000 - 0.5F;		// Standerd Speed
	SCI11.SIMR1.BYTE = 0x01;				// Simple I2C Mode
	SCI11.SIMR2.BYTE = 0x23;				// Syncronization, NACK Transmission
	SCI11.SCR.BYTE = 0xB4;					// TX/RX Enable TIE/TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
	MPC.PB6PFS.BYTE = 0x24;					// PB6 is SSCL11 Pin
	MPC.PB7PFS.BYTE = 0x24;					// PB7 is SSDA11 Pin
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
	PORTB.ODR1.BIT.B4 = 1;					// SSCL11 is Open Drain
	PORTB.PMR.BIT.B6 = 1;					// PB6 is Peripheral Pin
	PORTB.ODR1.BIT.B6 = 1;					// SSDA11 is Open Drain
	PORTB.PMR.BIT.B7 = 1;					// PB7 is Peripheral Pin
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci11_rxi11_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci11_rxi11_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI11, RXI11 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci11_txi11_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci11_txi11_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI11, TXI11 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT(SCI11, RXI11), SIIC_CFG_INT_PRIORITY );	// Enable SCI11 RXI11 Interrupt
	EnableInt( VECT(SCI11, TXI11), SIIC_CFG_INT_PRIORITY );	// Enable SCI11 TXI11 Interrupt
	DrvName = "siicl";					// Set Driver Name
	siic_tbl[SIIC11].siic = (void*)&SCI11;			// Set SCI Channel Address
	siic_tbl[SIIC11].next = MAXIMUM;			// Set Next Command Pointer

	u.t_cflg.flgatr = TA_TPRI | TA_WMUL;			// Set EventFlag Attribute
#if USE_OBJECT_NAME
	u.t_cflg.flgatr |= TA_DSNAME;				// Set EventFlag Attribute
#ifdef CLANGSPEC
	strcpy( u.t_cflg.dsname, "siic_f" );			// Set Debugger Suport Name
#else
	strcpy( (char*)u.t_cflg.dsname, "siic_f" );		// Set Debugger Suport Name
#endif /* CLANGSPEC */
#endif /* USE_OBJECT_NAME */
	u.t_cflg.iflgptn = 0;					// Set Initial Bit Pattern
	if( (objid = tk_cre_flg( &u.t_cflg )) <= E_OK )		// Create I2C EventFlag
		goto ERROR;
	siic_tbl[SIIC11].flgid = objid;				// Set I2C EventFlag ID

	u.t_ctsk.exinf = (void*)SIIC11;				// Set Exinf(SCI Channel Number)
	u.t_ctsk.tskatr  = TA_HLNG;				// Set Task Attribute
#if USE_OBJECT_NAME
	u.t_ctsk.tskatr |= TA_DSNAME;				// Set Task Attribute
#endif /* USE_OBJECT_NAME */
#if !USE_IMALLOC
	u.t_ctsk.tskatr |= TA_USERBUF;				// Set Task Attribute
	u.t_ctsk.bufptr = siic_task_stack[SIIC11];		// Set Stack Top Address
#endif /* USE_OBJECT_NAME */
	u.t_ctsk.stksz = 300;					// Set Task StackSize
	u.t_ctsk.itskpri = SIIC_CFG_TASK_PRIORITY;		// Set Task Priority
#ifdef CLANGSPEC
	u.t_ctsk.task = siic_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( u.t_ctsk.dsname, "siic_t" );			// Set Task Debugger Suport Name
#endif /* USE_OBJECT_NAME */
#else
	u.t_ctsk.task = (FP)siic_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)u.t_ctsk.dsname, "siic_t" );		// Set Task Debugger Suport Name
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( &u.t_ctsk )) <= E_OK )		// Create SIIC Control Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) < E_OK )			// Start SIIC Control Task
		goto ERROR;
	return siicDriverEntry( DrvName, SIIC11, &u.t_ddev );	// Define Device Driver
ERROR:
	while( 1 )  ;
}
