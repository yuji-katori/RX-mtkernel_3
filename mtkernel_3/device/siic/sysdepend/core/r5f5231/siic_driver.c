/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
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

typedef enum {
#ifdef USE_SIIC0
 SIIC0,
#endif /* USE_SIIC0 */
#ifdef USE_SIIC1
 SIIC1,
#endif /* USE_SIIC1 */
#ifdef USE_SIIC5
 SIIC5,
#endif /* USE_SIIC5 */
#ifdef USE_SIIC8
 SIIC8,
#endif /* USE_SIIC8 */
#ifdef USE_SIIC9
 SIIC9,
#endif /* USE_SIIC9 */
#ifdef USE_SIIC12
 SIIC12,
#endif /* USE_SIIC12 */
 OBJ_KIND_NUM } OBJ_KIND;
SIIC_TBL siic_tbl[OBJ_KIND_NUM];
#if !USE_IMALLOC
LOCAL INT siic_task_stack[OBJ_KIND_NUM][300/sizeof(INT)];
#endif /* USE_IMALLOC */

EXPORT ER SIIC_Read(T_DEVREQ *devreq, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UINT)exinf];
volatile struct st_sci0 __evenaccess *SIIC = siic->siic;
UINT flgptn;
ER ercd;
SZ i;
	SIIC->SIMR3.BYTE = 0x51;				// Generates Start Condition
	tk_wai_flg( siic->flgid, SIIC_STI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	SIIC->SIMR3.BYTE = 0x00;				// Clear Start Condistion
	SIIC->TDR = devreq->start << 1 | 0x01;			// Set Slave Address
	tk_wai_flg( siic->flgid, SIIC_TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	if( ! SIIC->SISR.BIT.IICACKR )  {			// Receive ACK
		SIIC->SIMR2.BIT.IICACKT = 0;			// Clear ACK Bit
		for( i=0 ; i<devreq->size ; i++ )  {
			if( i == devreq->size - 1 )		// Next Data is Last ?
				SIIC->SIMR2.BIT.IICACKT = 1;	// Set ACK Bit
			SIIC->SCR.BIT.RIE = 1;			// RIE Enable
			SIIC->TDR = 0xFF;			// Dummy Data Write and Wait RXI Interrupt
			tk_wai_flg( siic->flgid, SIIC_RXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
			((UB*)devreq->buf)[i] = SIIC->RDR;	// Data Read and Wait TXI Interrupt
			tk_wai_flg( siic->flgid, SIIC_TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		}
		devreq->asize = devreq->size;			// Set Write Size
		ercd = E_OK;					// Set Error Code (E_OK)
	}
	else  {							// Receive NACK
		devreq->asize = 0;				// Set Write Size
		ercd = E_IO;					// Set Error Code (E_IO:Receive NACK)
	}
	SIIC->SIMR3.BYTE = 0x54;				// Generates Stop Condition
	tk_wai_flg( siic->flgid, SIIC_STI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	SIIC->SIMR3.BYTE = 0xF0;				// Transmit End
	return ercd;
}

EXPORT ER SIIC_Write(T_DEVREQ *devreq, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UINT)exinf];
volatile struct st_sci0 __evenaccess *SIIC = siic->siic;
UINT flgptn;
ER ercd;
SZ i;
	SIIC->SIMR3.BYTE = 0x51;				// Generates Start Condition
	tk_wai_flg( siic->flgid, SIIC_STI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	SIIC->SIMR3.BYTE = 0x00;				// Clear Start Condistion
	SIIC->TDR = devreq->start << 1;				// Set Slave Address
	tk_wai_flg( siic->flgid, SIIC_TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	if( ! SIIC->SISR.BIT.IICACKR )  {			// Receive ACK
		for( i=0 ; i<devreq->size ; i++ )  {
			SIIC->TDR = ((UB*)devreq->buf)[i];	// Data Write and Wait TXI Interrupt
			tk_wai_flg( siic->flgid, SIIC_TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		}
		devreq->asize = devreq->size;			// Set Write Size
		ercd = E_OK;					// Set Error Code (E_OK)
	}
	else  {							// Receive NACK
		devreq->asize = 0;				// Set Write Size
		ercd = E_IO;					// Set Error Code (E_IO:Receive NACK)
	}
	SIIC->SIMR3.BYTE = 0x54;				// Generates Stop Condition
	tk_wai_flg( siic->flgid, SIIC_STI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	SIIC->SIMR3.BYTE = 0xF0;				// Transmit End
	return ercd;
}

LOCAL void SIIC_Handler(UINT ch, UINT flgptn)
{
SIIC_TBL *siic = &siic_tbl[ch];
	tk_set_flg( siic->flgid, flgptn );			// Set SIIC Event
}

#ifdef USE_SIIC0
LOCAL void sci0_rxi0_hdr(UINT dintno)
{
	SCI0.SCR.BIT.RIE = 0;					// SCI0 RIE Disable
	SIIC_Handler( SIIC0, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci0_txi0_hdr(UINT dintno)
{
	SIIC_Handler( SIIC0, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci0_tei0_hdr(UINT dintno)
{
	SCI0.SIMR3.BIT.IICSTIF = 0;				// Clear SCI0 IICSTIF of SIMR3
	SIIC_Handler( SIIC0, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC0 */

#ifdef USE_SIIC1
LOCAL void sci1_rxi1_hdr(UINT dintno)
{
	SCI1.SCR.BIT.RIE = 0;					// SCI1 RIE Disable
	SIIC_Handler( SIIC1, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci1_txi1_hdr(UINT dintno)
{
	SIIC_Handler( SIIC1, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci1_tei1_hdr(UINT dintno)
{
	SCI1.SIMR3.BIT.IICSTIF = 0;				// Clear SCI1 IICSTIF of SIMR3
	SIIC_Handler( SIIC1, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC1 */

#ifdef USE_SIIC5
LOCAL void sci5_rxi5_hdr(UINT dintno)
{
	SCI5.SCR.BIT.RIE = 0;					// SCI5 RIE Disable
	SIIC_Handler( SIIC5, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci5_txi5_hdr(UINT dintno)
{
	SIIC_Handler( SIIC5, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci5_tei5_hdr(UINT dintno)
{
	SCI5.SIMR3.BIT.IICSTIF = 0;				// Clear SCI5 IICSTIF of SIMR3
	SIIC_Handler( SIIC5, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC5 */

#ifdef USE_SIIC8
LOCAL void sci8_rxi8_hdr(UINT dintno)
{
	SCI8.SCR.BIT.RIE = 0;					// SCI8 RIE Disable
	SIIC_Handler( SIIC8, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci8_txi8_hdr(UINT dintno)
{
	SIIC_Handler( SIIC8, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci8_tei8_hdr(UINT dintno)
{
	SCI8.SIMR3.BIT.IICSTIF = 0;				// Clear SCI8 IICSTIF of SIMR3
	SIIC_Handler( SIIC8, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC8 */

#ifdef USE_SIIC9
LOCAL void sci9_rxi9_hdr(UINT dintno)
{
	SCI9.SCR.BIT.RIE = 0;					// SCI9 RIE Disable
	SIIC_Handler( SIIC9, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci9_txi9_hdr(UINT dintno)
{
	SIIC_Handler( SIIC9, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci9_tei9_hdr(UINT dintno)
{
	SCI9.SIMR3.BIT.IICSTIF = 0;				// Clear SCI9 IICSTIF of SIMR3
	SIIC_Handler( SIIC9, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC8 */

#ifdef USE_SIIC12
LOCAL void sci12_rxi12_hdr(UINT dintno)
{
	SCI12.SCR.BIT.RIE = 0;					// RIE Disable
	SIIC_Handler( SIIC12, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci12_txi12_hdr(UINT dintno)
{
	SIIC_Handler( SIIC12, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci12_tei12_hdr(UINT dintno)
{
	SCI12.SIMR3.BIT.IICSTIF = 0;				// Clear SCI11 IICSTIF of SIMR3
	SIIC_Handler( SIIC12, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC12 */

LOCAL ER siicCreFlg(SIIC_TBL *siic, T_CFLG *p_cflg)
{
	p_cflg->flgatr = TA_TPRI | TA_WMUL;			// Set EventFlag Attribute
#if USE_OBJECT_NAME
	p_cflg->flgatr |= TA_DSNAME;				// Set EventFlag Attribute
#ifdef CLANGSPEC
	strcpy( p_cflg->dsname, "siic*_f" );			// Set Debugger Suport Name
#else
	strcpy( (char*)p_cflg->dsname, "siic*_f" );		// Set Debugger Suport Name
#endif /* CLANGSPEC */
	p_cflg->dsname[4] = siic->drvname[4];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
	p_cflg->iflgptn = 0;					// Set Initial Bit Pattern
	return siic->flgid = tk_cre_flg( p_cflg );		// Create SIIC EventFlag
}

LOCAL ER siicCreTsk(SIIC_TBL *siic, UINT ch, T_CTSK *p_ctsk)
{
ID objid;
	p_ctsk->exinf = (void*)ch;				// Set Exinf(SCI Channel Number)
	p_ctsk->tskatr  = TA_HLNG;				// Set Task Attribute
#if USE_OBJECT_NAME
	p_ctsk->tskatr |= TA_DSNAME;				// Set Task Attribute
#endif /* USE_OBJECT_NAME */
#if !USE_IMALLOC
	p_ctsk->tskatr |= TA_USERBUF;				// Set Task Attribute
	p_ctsk->bufptr = siic_task_stack[ch];			// Set Stack Top Address
#endif /* USE_OBJECT_NAME */
	p_ctsk->stksz = 300;					// Set Task StackSize
	p_ctsk->itskpri = SIIC_CFG_TASK_PRIORITY;		// Set Task Priority
#ifdef CLANGSPEC
	p_ctsk->task = siic_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( p_ctsk->dsname, "siic*_t" );			// Set Task Debugger Suport Name
	p_ctsk->dsname[4] = siic->drvname[4];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
#else
	p_ctsk->task = (FP)siic_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)p_ctsk->dsname, "siic*_t" );		// Set Task Debugger Suport Name
	p_ctsk->dsname[4] = siic->drvname[4];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( p_ctsk )) <= E_OK )		// Create SIIC Control Task
		return objid;
	return tk_sta_tsk( objid, 0 );				// Start SIIC Control Task
}

EXPORT ER siicDrvEntry(void)
{
ER ercd;
union { T_CTSK t_ctsk; T_CFLG t_cflg; T_DDEV t_ddev; T_DINT t_dint; } u;
#ifdef USE_SIIC0
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI0 ) )  {					// SCI0 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI0 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI0 ) = 0;					// Enable SCI0
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI0.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI0.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI0.BRR = PCLKB / 32.0F / SIIC0_FSCL - 1;		// SCL Frequency
	SCI0.MDDR = (SCI0.BRR+1) * 8192.0F * SIIC0_FSCL / PCLKB;// Analize Modulation Duty
	if( SCI0.MDDR < 0x80 )					// Check  Modulation Duty
		return E_IO;
	SCI0.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI0.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI0.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI0.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI0.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL0_P21)
	MPC.P21PFS.BYTE = 0x0A;					// P21 is SSCL1 Pin
#endif
#if defined(USE_SSDA0_P20)
	MPC.P20PFS.BYTE = 0x0A;					// P20 is SSDA1 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL0_P21)
	PORT2.ODR0.BIT.B2 = 1;					// SSCL1 is Open Drain
	PORT2.PMR.BIT.B1 = 1;					// P21 is Peripheral Pin
#endif
#if defined(USE_SSDA0_P20)
	PORT2.ODR0.BIT.B0 = 1;					// SSDA1 is Open Drain
	PORT2.PMR.BIT.B0 = 1;					// P20 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci0_rxi0_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci0_rxi0_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI0, RXI0 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci0_txi0_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci0_txi0_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI0, TXI0 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci0_tei0_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci0_tei0_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI0, TEI0 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI0, RXI0 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI0 RXI0 Interrupt
	EnableInt( VECT( SCI0, TXI0 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI0 TXI0 Interrupt
	EnableInt( VECT( SCI0, TEI0 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI0 TEI0 Interrupt
	siic_tbl[SIIC0].drvname = "siica";			// Set Driver Name
	siic_tbl[SIIC0].siic = (void*)&SCI0;			// Set SCI Channel Address
	siic_tbl[SIIC0].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC0], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC0], SIIC0, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC0].drvname, SIIC0, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC0 */

#ifdef USE_SIIC1
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI1 ) )  {					// SCI1 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI1 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI1 ) = 0;					// Enable SCI1
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI1.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI1.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI1.BRR = PCLKB / 32.0F / SIIC1_FSCL - 1;		// SCL Frequency
	SCI1.MDDR = (SCI1.BRR+1) * 8192.0F * SIIC1_FSCL / PCLKB;// Analize Modulation Duty
	if( SCI1.MDDR < 0x80 )					// Check  Modulation Duty
		return E_IO;
	SCI1.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI1.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI1.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI1.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI1.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL1_P15)
	MPC.P15PFS.BYTE = 0x0A;					// P15 is SSCL1 Pin
#elif defined(USE_SSCL1_P30)
	MPC.P30PFS.BYTE = 0x0A;					// P30 is SSCL1 Pin
#endif
#if defined(USE_SSDA1_P16)
	MPC.P16PFS.BYTE = 0x0A;					// P16 is SSDA1 Pin
#elif defined(USE_SSDA1_P26)
	MPC.P26PFS.BYTE = 0x0A;					// P26 is SSDA1 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL1_P15)
	PORT1.ODR1.BIT.B2 = 1;					// SSCL1 is Open Drain
	PORT1.PMR.BIT.B5 = 1;					// P15 is Peripheral Pin
#elif defined(USE_SSCL1_P30)
	PORT3.ODR0.BIT.B0 = 1;					// SSCL1 is Open Drain
	PORT3.PMR.BIT.B0 = 1;					// P30 is Peripheral Pin
#endif
#if defined(USE_SSDA1_P16)
	PORT1.ODR1.BIT.B4 = 1;					// SSDA1 is Open Drain
	PORT1.PMR.BIT.B6 = 1;					// P16 is Peripheral Pin
#elif defined(USE_SSDA1_P26)
	PORT2.ODR1.BIT.B4 = 1;					// SSDA1 is Open Drain
	PORT2.PMR.BIT.B6 = 1;					// P26 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci1_rxi1_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci1_rxi1_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI1, RXI1 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci1_txi1_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci1_txi1_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI1, TXI1 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci1_tei1_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci1_tei1_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI1, TEI1 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI1, RXI1 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI1 RXI1 Interrupt
	EnableInt( VECT( SCI1, TXI1 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI1 TXI1 Interrupt
	EnableInt( VECT( SCI1, TEI1 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI1 TEI1 Interrupt
	siic_tbl[SIIC1].drvname = "siicb";			// Set Driver Name
	siic_tbl[SIIC1].siic = (void*)&SCI1;			// Set SCI Channel Address
	siic_tbl[SIIC1].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC1], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC1], SIIC1, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC1].drvname, SIIC1, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC1 */

#ifdef USE_SIIC5
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI5 ) )  {					// SCI5 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI5 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI5 ) = 0;					// Enable SCI5
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI5.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI5.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI5.BRR = PCLKB / 32.0F / SIIC5_FSCL - 1;		// SCL Frequency
	SCI5.MDDR = (SCI5.BRR+1) * 8192.0F * SIIC5_FSCL / PCLKB;// Analize Modulation Duty
	if( SCI5.MDDR < 0x80 )					// Check  Modulation Duty
		return E_IO;
	SCI5.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI5.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI5.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI5.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI5.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL5_PA2)
	MPC.PA2PFS.BYTE = 0x0A;					// PA2 is SSCL5 Pin
#elif defined(USE_SSCL5_PA3)
	MPC.PA3PFS.BYTE = 0x0A;					// PA3 is SSCL5 Pin
#elif defined(USE_SSCL5_PC2)
	MPC.PC2PFS.BYTE = 0x0A;					// PC2 is SSCL5 Pin
#endif
#if defined(USE_SSDA5_PA4)
	MPC.PA4PFS.BYTE = 0x0A;					// PA4 is SSDA5 Pin
#elif defined(USE_SSDA5_PC3)
	MPC.PC3PFS.BYTE = 0x0A;					// PC3 is SSDA5 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL5_PA2)
	PORTA.ODR0.BIT.B4 = 1;					// SSCL5 is Open Drain
	PORTA.PMR.BIT.B2 = 1;					// PA2 is Peripheral Pin
#elif defined(USE_SSCL5_PA3)
	PORTA.ODR0.BIT.B6 = 1;					// SSCL5 is Open Drain
	PORTA.PMR.BIT.B3 = 1;					// PA3 is Peripheral Pin
#elif defined(USE_SSCL5_PC2)
	PORTC.ODR0.BIT.B4 = 1;					// SSCL5 is Open Drain
	PORTC.PMR.BIT.B2 = 1;					// PC2 is Peripheral Pin
#endif
#if defined(USE_SSDA5_PA4)
	PORTA.ODR1.BIT.B0 = 1;					// SSDA5 is Open Drain
	PORTA.PMR.BIT.B4 = 1;					// PA4 is Peripheral Pin
#elif defined(USE_SSDA5_PC3)
	PORTC.ODR0.BIT.B6 = 1;					// SSDA5 is Open Drain
	PORTC.PMR.BIT.B3 = 1;					// PC3 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci5_rxi5_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci5_rxi5_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI5, RXI5 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci5_txi5_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci5_txi5_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI5, TXI5 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci5_tei5_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci5_tei5_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI5, TEI5 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI5, RXI5 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI5 RXI5 Interrupt
	EnableInt( VECT( SCI5, TXI5 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI5 TXI5 Interrupt
	EnableInt( VECT( SCI5, TEI5 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI5 TEI5 Interrupt
	siic_tbl[SIIC5].drvname = "siicf";			// Set Driver Name
	siic_tbl[SIIC5].siic = (void*)&SCI5;			// Set SCI Channel Address
	siic_tbl[SIIC5].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC5], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC5], SIIC5, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC5].drvname, SIIC5, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC5 */

#ifdef USE_SIIC8
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI8 ) )  {					// SCI8 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI8 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI8 ) = 0;					// Enable SCI8
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI8.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI8.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI8.BRR = PCLKB / 32.0F / SIIC8_FSCL - 1;		// SCL Frequency
	SCI8.MDDR = (SCI8.BRR+1) * 8192.0F * SIIC8_FSCL / PCLKB;// Analize Modulation Duty
	if( SCI8.MDDR < 0x80 )					// Check  Modulation Duty
		return E_IO;
	SCI8.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI8.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI8.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI8.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI8.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL8_PC6)
	MPC.PC6PFS.BYTE = 0x0A;					// PC6 is SSCL8 Pin
#endif
#if defined(USE_SSDA8_PC7)
	MPC.PC7PFS.BYTE = 0x0A;					// PC7 is SSDA8 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL8_PC6)
	PORTC.ODR1.BIT.B4 = 1;					// SSCL8 is Open Drain
	PORTC.PMR.BIT.B6 = 1;					// PC6 is Peripheral Pin
#endif
#if defined(USE_SSDA8_PC7)
	PORTC.ODR1.BIT.B6 = 1;					// SSDA8 is Open Drain
	PORTC.PMR.BIT.B7 = 1;					// PC7 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci8_rxi8_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci8_rxi8_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI8, RXI8 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci8_txi8_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci8_txi8_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI8, TXI8 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci8_tei8_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci8_tei8_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI8, TEI8 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI8, RXI8 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI8 RXI8 Interrupt
	EnableInt( VECT( SCI8, TXI8 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI8 TXI8 Interrupt
	EnableInt( VECT( SCI8, TEI8 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI8 TEI8 Interrupt
	siic_tbl[SIIC8].drvname = "siici";			// Set Driver Name
	siic_tbl[SIIC8].siic = (void*)&SCI8;			// Set SCI Channel Address
	siic_tbl[SIIC8].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC8], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC8], SIIC8, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC8].drvname, SIIC8, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC8 */

#ifdef USE_SIIC9
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI9 ) )  {					// SCI9 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI9 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI9 ) = 0;					// Enable SCI9
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI9.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI9.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI9.BRR = PCLKB / 32.0F / SIIC9_FSCL - 1;		// SCL Frequency
	SCI9.MDDR = (SCI9.BRR+1) * 8192.0F * SIIC9_FSCL / PCLKB;// Analize Modulation Duty
	if( SCI9.MDDR < 0x80 )					// Check  Modulation Duty
		return E_IO;
	SCI9.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI9.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI9.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI9.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI9.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL9_PB6)
	MPC.PB6PFS.BYTE = 0x0A;					// PB6 is SSCL9 Pin
#endif
#if defined(USE_SSDA9_PB7)
	MPC.PB7PFS.BYTE = 0x0A;					// PB7 is SSDA9 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL9_PB6)
	PORTB.ODR1.BIT.B4 = 1;					// SSCL9 is Open Drain
	PORTB.PMR.BIT.B6 = 1;					// PB6 is Peripheral Pin
#endif
#if defined(USE_SSDA9_PB7)
	PORTB.ODR1.BIT.B6 = 1;					// SSDA9 is Open Drain
	PORTB.PMR.BIT.B7 = 1;					// PB7 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci9_rxi9_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci9_rxi9_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI9, RXI9 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci9_txi9_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci9_txi9_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI9, TXI9 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci9_tei9_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci9_tei9_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI9, TEI9 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI9, RXI9 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI9 RXI9 Interrupt
	EnableInt( VECT( SCI9, TXI9 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI9 TXI9 Interrupt
	EnableInt( VECT( SCI9, TEI9 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI9 TEI9 Interrupt
	siic_tbl[SIIC9].drvname = "siicj";			// Set Driver Name
	siic_tbl[SIIC9].siic = (void*)&SCI9;			// Set SCI Channel Address
	siic_tbl[SIIC9].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC9], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC9], SIIC9, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC9].drvname, SIIC9, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC9 */

#ifdef USE_SIIC12
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI12 ) )  {				// SCI12 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI12 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI12 ) = 0;					// Enable SCI12
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI12.SIMR3.BYTE = 0xF0;				// SSCL,SSDA to High Impedance
	SCI12.SCMR.BIT.SDIR = 1;				// MSB First Transmit
	SCI12.BRR = PCLKB / 32.0F / SIIC12_FSCL - 1;		// SCL Frequency
	SCI12.MDDR = (SCI12.BRR+1)*8192.0F*SIIC12_FSCL/PCLKB;	// Analize Modulation Duty
	if( SCI12.MDDR < 0x80 )					// Check  Modulation Duty
		return E_IO;
	SCI12.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI12.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI12.SIMR1.BYTE = 0x01;				// Simple I2C Mode
	SCI12.SIMR2.BYTE = 0x23;				// Syncronization, NACK Transmission
	SCI12.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL12_PE2)
	MPC.PE2PFS.BYTE = 0x0C;					// PE2 is SSCL12 Pin
#endif
#if defined(USE_SSDA12_PE1)
	MPC.PE1PFS.BYTE = 0x0C;					// PE1 is SSDA12 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL12_PE2)
	PORTE.ODR0.BIT.B4 = 1;					// SSCL12 is Open Drain
	PORTE.PMR.BIT.B2 = 1;					// PE2 is Peripheral Pin
#endif
#if defined(USE_SSDA12_PE1)
	PORTE.ODR0.BIT.B2 = 1;					// SSDA12 is Open Drain
	PORTE.PMR.BIT.B1 = 1;					// PE1 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci12_rxi12_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci12_rxi12_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI12, RXI12 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci12_txi12_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci12_txi12_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI12, TXI12 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci12_tei12_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci12_tei12_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI12, TEI12 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT(SCI12, RXI12), SIIC_CFG_INT_PRIORITY );	// Enable SCI12 RXI12 Interrupt
	EnableInt( VECT(SCI12, TXI12), SIIC_CFG_INT_PRIORITY );	// Enable SCI12 TXI12 Interrupt
	EnableInt( VECT(SCI12, TEI12), SIIC_CFG_INT_PRIORITY );	// Enable SCI12 TEI12 Interrupt
	siic_tbl[SIIC12].drvname = "siicm";			// Set Driver Name
	siic_tbl[SIIC12].siic = (void*)&SCI12;			// Set SCI Channel Address
	siic_tbl[SIIC12].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC12], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC12], SIIC12, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC12].drvname, SIIC12, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC12 */
	return E_OK;
ERROR:
	return ercd;
}