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
#ifdef USE_SIIC2
 SIIC2,
#endif /* USE_SIIC2 */
#ifdef USE_SIIC3
 SIIC3,
#endif /* USE_SIIC3 */
#ifdef USE_SIIC4
 SIIC4,
#endif /* USE_SIIC4 */
#ifdef USE_SIIC5
 SIIC5,
#endif /* USE_SIIC5 */
#ifdef USE_SIIC6
 SIIC6,
#endif /* USE_SIIC6 */
#ifdef USE_SIIC7
 SIIC7,
#endif /* USE_SIIC7 */
#ifdef USE_SIIC8
 SIIC8,
#endif /* USE_SIIC8 */
#ifdef USE_SIIC9
 SIIC9,
#endif /* USE_SIIC9 */
#ifdef USE_SIIC10
 SIIC10,
#endif /* USE_SIIC10 */
#ifdef USE_SIIC11
 SIIC11,
#endif /* USE_SIIC11 */
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

#ifdef USE_SIIC2
LOCAL void sci2_rxi2_hdr(UINT dintno)
{
	SCI2.SCR.BIT.RIE = 0;					// SCI2 RIE Disable
	SIIC_Handler( SIIC2, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci2_txi2_hdr(UINT dintno)
{
	SIIC_Handler( SIIC2, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci2_tei2_hdr(UINT dintno)
{
	SCI2.SIMR3.BIT.IICSTIF = 0;				// Clear SCI2 IICSTIF of SIMR3
	SIIC_Handler( SIIC2, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC2 */

#ifdef USE_SIIC3
LOCAL void sci3_rxi3_hdr(UINT dintno)
{
	SCI3.SCR.BIT.RIE = 0;					// SCI3 RIE Disable
	SIIC_Handler( SIIC3, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci3_txi3_hdr(UINT dintno)
{
	SIIC_Handler( SIIC3, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci3_tei3_hdr(UINT dintno)
{
	SCI3.SIMR3.BIT.IICSTIF = 0;				// Clear SCI3 IICSTIF of SIMR3
	SIIC_Handler( SIIC3, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC3 */

#ifdef USE_SIIC4
LOCAL void sci4_rxi4_hdr(UINT dintno)
{
	SCI4.SCR.BIT.RIE = 0;					// SCI4 RIE Disable
	SIIC_Handler( SIIC4, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci4_txi4_hdr(UINT dintno)
{
	SIIC_Handler( SIIC4, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci4_tei4_hdr(UINT dintno)
{
	SCI4.SIMR3.BIT.IICSTIF = 0;				// Clear SCI4 IICSTIF of SIMR3
	SIIC_Handler( SIIC4, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC4 */

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

#ifdef USE_SIIC6
LOCAL void sci6_rxi6_hdr(UINT dintno)
{
	SCI6.SCR.BIT.RIE = 0;					// SCI6 RIE Disable
	SIIC_Handler( SIIC6, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci6_txi6_hdr(UINT dintno)
{
	SIIC_Handler( SIIC6, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci6_tei6_hdr(UINT dintno)
{
	SCI6.SIMR3.BIT.IICSTIF = 0;				// Clear SCI6 IICSTIF of SIMR3
	SIIC_Handler( SIIC6, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC6 */

#ifdef USE_SIIC7
LOCAL void sci7_rxi7_hdr(UINT dintno)
{
	SCI7.SCR.BIT.RIE = 0;					// SCI7 RIE Disable
	SIIC_Handler( SIIC7, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci7_txi7_hdr(UINT dintno)
{
	SIIC_Handler( SIIC7, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci7_tei7_hdr(UINT dintno)
{
	SCI7.SIMR3.BIT.IICSTIF = 0;				// Clear SCI7 IICSTIF of SIMR3
	SIIC_Handler( SIIC7, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC7 */

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
#endif /* USE_SIIC9 */

#ifdef USE_SIIC10
LOCAL void sci10_rxi10_hdr(UINT dintno)
{
	SCI10.SCR.BIT.RIE = 0;					// RIE Disable
	SIIC_Handler( SIIC10, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci10_txi10_hdr(UINT dintno)
{
	SIIC_Handler( SIIC10, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci10_tei10_hdr(UINT dintno)
{
	SCI10.SIMR3.BIT.IICSTIF = 0;				// Clear SCI10 IICSTIF of SIMR3
	SIIC_Handler( SIIC10, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC10 */

#ifdef USE_SIIC11
LOCAL void sci11_rxi11_hdr(UINT dintno)
{
	SCI11.SCR.BIT.RIE = 0;					// RIE Disable
	SIIC_Handler( SIIC11, SIIC_RXI_INT );			// Call SIIC_Handler
}

LOCAL void sci11_txi11_hdr(UINT dintno)
{
	SIIC_Handler( SIIC11, SIIC_TXI_INT );			// Call SIIC_Handler
}

LOCAL void sci11_tei11_hdr(UINT dintno)
{
	SCI11.SIMR3.BIT.IICSTIF = 0;				// Clear SCI11 IICSTIF of SIMR3
	SIIC_Handler( SIIC11, SIIC_STI_INT );			// Call SIIC_Handler
}
#endif /* USE_SIIC11 */

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
	SCI0.BRR = PCLK / 32.0F / SIIC0_FSCL - 1;		// SCL Frequency
	SCI0.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI0.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI0.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI0.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI0.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL0_P21)
	MPC.P21PFS.BYTE = 0x0A;					// P21 is SSCL0 Pin
#elif defined(USE_SSCL0_P33)
	MPC.P33PFS.BYTE = 0x0B;					// P33 is SSCL0 Pin
#endif
#if defined(USE_SSDA0_P20)
	MPC.P20PFS.BYTE = 0x0A;					// P20 is SSDA0 Pin
#elif defined(USE_SSDA0_P32)
	MPC.P32PFS.BYTE = 0x0B;					// P32 is SSDA0 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL0_P21)
	PORT2.ODR0.BIT.B2 = 1;					// SSCL0 is Open Drain
	PORT2.PMR.BIT.B1 = 1;					// P21 is Peripheral Pin
#elif defined(USE_SSCL0_P33)
	PORT3.ODR0.BIT.B6 = 1;					// SSCL0 is Open Drain
	PORT3.PMR.BIT.B3 = 1;					// P33 is Peripheral Pin
#endif
#if defined(USE_SSDA0_P20)
	PORT2.ODR0.BIT.B0 = 1;					// SSDA0 is Open Drain
	PORT2.PMR.BIT.B0 = 1;					// P20 is Peripheral Pin
#elif defined(USE_SSDA0_P32)
	PORT3.ODR0.BIT.B4 = 1;					// SSDA0 is Open Drain
	PORT3.PMR.BIT.B2 = 1;					// P32 is Peripheral Pin
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
	SCI1.BRR = PCLK / 32.0F / SIIC1_FSCL - 1;		// SCL Frequency
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
#elif defined(USE_SSCL1_PF2)
	MPC.PF2PFS.BYTE = 0x0A;					// PF2 is SSCL1 Pin
#endif
#if defined(USE_SSDA1_P16)
	MPC.P16PFS.BYTE = 0x0A;					// P16 is SSDA1 Pin
#elif defined(USE_SSDA1_P26)
	MPC.P26PFS.BYTE = 0x0A;					// P26 is SSDA1 Pin
#elif defined(USE_SSDA1_PF0)
	MPC.PF0PFS.BYTE = 0x0A;					// PF0 is SSDA1 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL1_P15)
	PORT1.ODR1.BIT.B2 = 1;					// SSCL1 is Open Drain
	PORT1.PMR.BIT.B5 = 1;					// P15 is Peripheral Pin
#elif defined(USE_SSCL1_P30)
	PORT3.ODR0.BIT.B0 = 1;					// SSCL1 is Open Drain
	PORT3.PMR.BIT.B0 = 1;					// P30 is Peripheral Pin
#elif defined(USE_SSCL1_PF2)
	PORTF.ODR0.BIT.B4 = 1;					// SSCL1 is Open Drain
	PORTF.PMR.BIT.B2 = 1;					// PF2 is Peripheral Pin
#endif
#if defined(USE_SSDA1_P16)
	PORT1.ODR1.BIT.B4 = 1;					// SSDA1 is Open Drain
	PORT1.PMR.BIT.B6 = 1;					// P16 is Peripheral Pin
#elif defined(USE_SSDA1_P26)
	PORT2.ODR1.BIT.B4 = 1;					// SSDA1 is Open Drain
	PORT2.PMR.BIT.B6 = 1;					// P26 is Peripheral Pin
#elif defined(USE_SSDA1_PF0)
	PORTF.ODR0.BIT.B0 = 1;					// SSDA1 is Open Drain
	PORTF.PMR.BIT.B0 = 1;					// PF0 is Peripheral Pin
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

#ifdef USE_SIIC2
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI2 ) )  {					// SCI2 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI2 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI2 ) = 0;					// Enable SCI2
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI2.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI2.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI2.BRR = PCLK / 32.0F / SIIC2_FSCL - 1;		// SCL Frequency
	SCI2.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI2.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI2.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI2.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI2.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL2_P12)
	MPC.P12PFS.BYTE = 0x0A;					// P12 is SSCL2 Pin
#elif defined(USE_SSCL2_P52)
	MPC.P52PFS.BYTE = 0x0A;					// P52 is SSCL2 Pin
#endif
#if defined(USE_SSDA2_P13)
	MPC.P13PFS.BYTE = 0x0A;					// P13 is SSDA2 Pin
#elif defined(USE_SSDA2_P50)
	MPC.P50PFS.BYTE = 0x0A;					// P50 is SSDA2 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL2_P12)
	PORT1.ODR0.BIT.B4 = 1;					// SSCL2 is Open Drain
	PORT1.PMR.BIT.B2 = 1;					// P12 is Peripheral Pin
#elif defined(USE_SSCL2_P52)
	PORT5.ODR0.BIT.B4 = 1;					// SSCL2 is Open Drain
	PORT5.PMR.BIT.B2 = 1;					// P50 is Peripheral Pin
#endif
#if defined(USE_SSDA2_P13)
	PORT1.ODR0.BIT.B6 = 1;					// SSDA2 is Open Drain
	PORT1.PMR.BIT.B3 = 1;					// P13 is Peripheral Pin
#elif defined(USE_SSDA2_P50)
	PORT5.ODR0.BIT.B0 = 1;					// SSDA2 is Open Drain
	PORT5.PMR.BIT.B0 = 1;					// P50 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci2_rxi2_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci2_rxi2_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI2, RXI2 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci2_txi2_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci2_txi2_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI2, TXI2 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci2_tei2_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci2_tei2_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI2, TEI2 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI2, RXI2 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI2 RXI2 Interrupt
	EnableInt( VECT( SCI2, TXI2 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI2 TXI2 Interrupt
	EnableInt( VECT( SCI2, TEI2 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI2 TEI2 Interrupt
	siic_tbl[SIIC2].drvname = "siicc";			// Set Driver Name
	siic_tbl[SIIC2].siic = (void*)&SCI2;			// Set SCI Channel Address
	siic_tbl[SIIC2].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC2], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC2], SIIC2, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC2].drvname, SIIC2, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC2 */

#ifdef USE_SIIC3
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI3 ) )  {					// SCI3 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI3 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI3 ) = 0;					// Enable SCI3
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI3.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI3.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI3.BRR = PCLK / 32.0F / SIIC3_FSCL - 1;		// SCL Frequency
	SCI3.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI3.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI3.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI3.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI3.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL3_P16)
	MPC.P16PFS.BYTE = 0x0B;					// P16 is SSCL3 Pin
#elif defined(USE_SSCL3_P25)
	MPC.P25PFS.BYTE = 0x0A;					// P25 is SSCL3 Pin
#endif
#if defined(USE_SSDA3_P17)
	MPC.P17PFS.BYTE = 0x0B;					// P17 is SSDA3 Pin
#elif defined(USE_SSDA3_P23)
	MPC.P23PFS.BYTE = 0x0A;					// P23 is SSDA3 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL3_P16)
	PORT1.ODR1.BIT.B4 = 1;					// SSCL3 is Open Drain
	PORT1.PMR.BIT.B6 = 1;					// P16 is Peripheral Pin
#elif defined(USE_SSCL3_P25)
	PORT2.ODR1.BIT.B2 = 1;					// SSCL3 is Open Drain
	PORT2.PMR.BIT.B5 = 1;					// P25 is Peripheral Pin
#endif
#if defined(USE_SSDA3_P17)
	PORT1.ODR1.BIT.B6 = 1;					// SSDA3 is Open Drain
	PORT1.PMR.BIT.B7 = 1;					// P17 is Peripheral Pin
#elif defined(USE_SSDA3_P23)
	PORT2.ODR0.BIT.B6 = 1;					// SSDA3 is Open Drain
	PORT2.PMR.BIT.B3 = 1;					// P23 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci3_rxi3_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci3_rxi3_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI3, RXI3 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci3_txi3_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci3_txi3_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI3, TXI3 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci3_tei3_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci3_tei3_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI3, TEI3 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI3, RXI3 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI3 RXI3 Interrupt
	EnableInt( VECT( SCI3, TXI3 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI3 TXI3 Interrupt
	EnableInt( VECT( SCI3, TEI3 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI3 TEI3 Interrupt
	siic_tbl[SIIC3].drvname = "siicd";			// Set Driver Name
	siic_tbl[SIIC3].siic = (void*)&SCI3;			// Set SCI Channel Address
	siic_tbl[SIIC3].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC3], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC3], SIIC3, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC3].drvname, SIIC3, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC3 */

#ifdef USE_SIIC4
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI4 ) )  {					// SCI4 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI4 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI4 ) = 0;					// Enable SCI4
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI4.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI4.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI4.BRR = PCLK / 32.0F / SIIC4_FSCL - 1;		// SCL Frequency
	SCI4.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI4.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI4.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI4.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI4.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL4_PB0)
	MPC.PB0PFS.BYTE = 0x0A;					// PB0 is SSCL4 Pin
#endif
#if defined(USE_SSDA4_PB1)
	MPC.PB1PFS.BYTE = 0x0A;					// PB1 is SSDA4 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL4_PB0)
	PORTB.ODR0.BIT.B0 = 1;					// SSCL4 is Open Drain
	PORTB.PMR.BIT.B0 = 1;					// PB0 is Peripheral Pin
#endif
#if defined(USE_SSDA4_PB1)
	PORTB.ODR0.BIT.B2 = 1;					// SSDA4 is Open Drain
	PORTB.PMR.BIT.B1 = 1;					// PB1 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci4_rxi4_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci4_rxi4_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI4, RXI4 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci4_txi4_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci4_txi4_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI4, TXI4 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci4_tei4_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci4_tei4_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI4, TEI4 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI4, RXI4 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI4 RXI4 Interrupt
	EnableInt( VECT( SCI4, TXI4 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI4 TXI4 Interrupt
	EnableInt( VECT( SCI4, TEI4 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI4 TEI4 Interrupt
	siic_tbl[SIIC4].drvname = "siice";			// Set Driver Name
	siic_tbl[SIIC4].siic = (void*)&SCI4;			// Set SCI Channel Address
	siic_tbl[SIIC4].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC4], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC4], SIIC4, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC4].drvname, SIIC4, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC4 */

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
	SCI5.BRR = PCLK / 32.0F / SIIC5_FSCL - 1;		// SCL Frequency
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

#ifdef USE_SIIC6
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI6 ) )  {					// SCI6 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI6 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI6 ) = 0;					// Enable SCI6
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI6.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI6.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI6.BRR = PCLK / 32.0F / SIIC6_FSCL - 1;		// SCL Frequency
	SCI6.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI6.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI6.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI6.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI6.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL6_P01)
	MPC.P01PFS.BYTE = 0x0A;					// P01 is SSCL6 Pin
#elif defined(USE_SSCL6_P33)
	MPC.P33PFS.BYTE = 0x0A;					// P33 is SSCL6 Pin
#elif defined(USE_SSCL6_PB0)
	MPC.PB0PFS.BYTE = 0x0B;					// PB0 is SSCL6 Pin
#endif
#if defined(USE_SSDA6_P00)
	MPC.P00PFS.BYTE = 0x0A;					// P00 is SSDA6 Pin
#elif defined(USE_SSDA6_P32)
	MPC.P32PFS.BYTE = 0x0A;					// P32 is SSDA6 Pin
#elif defined(USE_SSDA6_PB1)
	MPC.PB1PFS.BYTE = 0x0B;					// PB1 is SSDA6 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL6_P01)
	PORT0.ODR0.BIT.B2 = 1;					// SSCL6 is Open Drain
	PORT0.PMR.BIT.B1 = 1;					// P01 is Peripheral Pin
#elif defined(USE_SSCL6_P33)
	PORT3.ODR0.BIT.B6 = 1;					// SSCL6 is Open Drain
	PORT3.PMR.BIT.B3 = 1;					// P33 is Peripheral Pin
#elif defined(USE_SSCL6_PB0)
	PORTB.ODR0.BIT.B0 = 1;					// SSCL6 is Open Drain
	PORTB.PMR.BIT.B0 = 1;					// PB0 is Peripheral Pin
#endif
#if defined(USE_SSDA6_P00)
	PORT0.ODR0.BIT.B0 = 1;					// SSDA6 is Open Drain
	PORT0.PMR.BIT.B0 = 1;					// P00 is Peripheral Pin
#elif defined(USE_SSDA6_P32)
	PORT3.ODR0.BIT.B4 = 1;					// SSDA6 is Open Drain
	PORT3.PMR.BIT.B2 = 1;					// P32 is Peripheral Pin
#elif defined(USE_SSDA6_PB1)
	PORTB.ODR0.BIT.B2 = 1;					// SSDA6 is Open Drain
	PORTB.PMR.BIT.B1 = 1;					// PB1 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci6_rxi6_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci6_rxi6_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI6, RXI6 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci6_txi6_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci6_txi6_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI6, TXI6 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci6_tei6_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci6_tei6_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI6, TEI6 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI6, RXI6 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI6 RXI6 Interrupt
	EnableInt( VECT( SCI6, TXI6 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI6 TXI6 Interrupt
	EnableInt( VECT( SCI6, TEI6 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI6 TEI6 Interrupt
	siic_tbl[SIIC6].drvname = "siicg";			// Set Driver Name
	siic_tbl[SIIC6].siic = (void*)&SCI6;			// Set SCI Channel Address
	siic_tbl[SIIC6].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC6], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC6], SIIC6, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC6].drvname, SIIC6, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC6 */

#ifdef USE_SIIC7
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI7 ) )  {					// SCI7 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI7 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI7 ) = 0;					// Enable SCI7
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI7.SIMR3.BYTE = 0xF0;					// SSCL,SSDA to High Impedance
	SCI7.SCMR.BIT.SDIR = 1;					// MSB First Transmit
	SCI7.BRR = PCLK / 32.0F / SIIC7_FSCL - 1;		// SCL Frequency
	SCI7.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI7.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI7.SIMR1.BYTE = 0x01;					// Simple I2C Mode
	SCI7.SIMR2.BYTE = 0x23;					// Syncronization, NACK Transmission
	SCI7.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL7_P92)
	MPC.P92PFS.BYTE = 0x0A;					// P92 is SSCL7 Pin
#endif
#if defined(USE_SSDA7_P90)
	MPC.P90PFS.BYTE = 0x0A;					// P90 is SSDA7 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL7_P92)
	PORT9.ODR0.BIT.B4 = 1;					// SSCL7 is Open Drain
	PORT9.PMR.BIT.B2 = 1;					// P92 is Peripheral Pin
#endif
#if defined(USE_SSDA7_P90)
	PORT9.ODR0.BIT.B0 = 1;					// SSDA7 is Open Drain
	PORT9.PMR.BIT.B0 = 1;					// P90 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci7_rxi7_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci7_rxi7_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI7, RXI7 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci7_txi7_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci7_txi7_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI7, TXI7 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci7_tei7_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci7_tei7_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI7, TEI7 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI7, RXI7 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI7 RXI7 Interrupt
	EnableInt( VECT( SCI7, TXI7 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI7 TXI7 Interrupt
	EnableInt( VECT( SCI7, TEI7 ), SIIC_CFG_INT_PRIORITY );	// Enable SCI7 TEI7 Interrupt
	siic_tbl[SIIC7].drvname = "siich";			// Set Driver Name
	siic_tbl[SIIC7].siic = (void*)&SCI6;			// Set SCI Channel Address
	siic_tbl[SIIC7].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC7], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC7], SIIC7, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC7].drvname, SIIC7, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC7 */

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
	SCI8.BRR = PCLK / 32.0F / SIIC8_FSCL - 1;		// SCL Frequency
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
	SCI9.BRR = PCLK / 32.0F / SIIC9_FSCL - 1;		// SCL Frequency
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

#ifdef USE_SIIC10
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI10 ) )  {				// SCI10 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI10 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI10 ) = 0;					// Enable SCI10
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	SCI10.SIMR3.BYTE = 0xF0;				// SSCL,SSDA to High Impedance
	SCI10.SCMR.BIT.SDIR = 1;				// MSB First Transmit
	SCI10.BRR = PCLK / 32.0F / SIIC10_FSCL - 1;		// SCL Frequency
	SCI10.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI10.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI10.SIMR1.BYTE = 0x01;				// Simple I2C Mode
	SCI10.SIMR2.BYTE = 0x23;				// Syncronization, NACK Transmission
	SCI10.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL10_P81)
	MPC.P81PFS.BYTE = 0x0A;					// P81 is SSCL10 Pin
#endif
#if defined(USE_SSDA10_P82)
	MPC.P82PFS.BYTE = 0x0A;					// P82 is SSDA10 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL10_P81)
	PORT8.ODR0.BIT.B2 = 1;					// SSCL10 is Open Drain
	PORT8.PMR.BIT.B1 = 1;					// P81 is Peripheral Pin
#endif
#if defined(USE_SSDA10_P82)
	PORT8.ODR0.BIT.B4 = 1;					// SSDA10 is Open Drain
	PORT8.PMR.BIT.B2 = 1;					// P81 is Peripheral Pin
#endif
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci10_rxi10_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci10_rxi10_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI10, RXI10 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci10_txi10_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci10_txi10_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI10, TXI10 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci10_tei10_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci10_tei10_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI10, TEI10 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT(SCI10, RXI10), SIIC_CFG_INT_PRIORITY );	// Enable SCI10 RXI10 Interrupt
	EnableInt( VECT(SCI10, TXI10), SIIC_CFG_INT_PRIORITY );	// Enable SCI10 TXI10 Interrupt
	EnableInt( VECT(SCI10, TEI10), SIIC_CFG_INT_PRIORITY );	// Enable SCI10 TEI10 Interrupt
	siic_tbl[SIIC10].drvname = "siick";			// Set Driver Name
	siic_tbl[SIIC10].siic = (void*)&SCI10;			// Set SCI Channel Address
	siic_tbl[SIIC10].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC10], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC10], SIIC10, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC10].drvname, SIIC10, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC10 */

#ifdef USE_SIIC11
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
	SCI11.BRR = PCLK / 32.0F / SIIC11_FSCL - 1;		// SCL Frequency
	SCI11.SEMR.BYTE = 0x24;					// NFEN,BRME Enable
	SCI11.SNFR.BYTE = 0x01;					// 1 Use with Noise Filter
	SCI11.SIMR1.BYTE = 0x01;				// Simple I2C Mode
	SCI11.SIMR2.BYTE = 0x23;				// Syncronization, NACK Transmission
	SCI11.SCR.BYTE = 0xB4;					// TX/RX Enable TIE,TEIE Enable
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_SSCL11_P76)
	MPC.P76PFS.BYTE = 0x0A;					// P76 is SSCL11 Pin
#endif
#if defined(USE_SSDA11_P77)
	MPC.P77PFS.BYTE = 0x0A;					// P77 is SSDA11 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_SSCL11_P76)
	PORT7.ODR1.BIT.B4 = 1;					// SSCL11 is Open Drain
	PORT7.PMR.BIT.B6 = 1;					// P76 is Peripheral Pin
#endif
#if defined(USE_SSDA11_P77)
	PORT7.ODR1.BIT.B6 = 1;					// SSDA11 is Open Drain
	PORT7.PMR.BIT.B7 = 1;					// P77 is Peripheral Pin
#endif
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci11_tei11_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci11_tei11_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI11, TEI11 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT(SCI11, RXI11), SIIC_CFG_INT_PRIORITY );	// Enable SCI11 RXI11 Interrupt
	EnableInt( VECT(SCI11, TXI11), SIIC_CFG_INT_PRIORITY );	// Enable SCI11 TXI11 Interrupt
	EnableInt( VECT(SCI11, TEI11), SIIC_CFG_INT_PRIORITY );	// Enable SCI11 TEI11 Interrupt
	siic_tbl[SIIC11].drvname = "siicl";			// Set Driver Name
	siic_tbl[SIIC11].siic = (void*)&SCI11;			// Set SCI Channel Address
	siic_tbl[SIIC11].next = SIIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = siicCreFlg( &siic_tbl[SIIC11], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SIIC EventFlag
	if( ( ercd = siicCreTsk( &siic_tbl[SIIC11], SIIC11, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SIIC Task
	if( ( ercd = siicDriverEntry( siic_tbl[SIIC11].drvname, SIIC11, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SIIC11 */

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
	SCI12.BRR = PCLK / 32.0F / SIIC12_FSCL - 1;		// SCL Frequency
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