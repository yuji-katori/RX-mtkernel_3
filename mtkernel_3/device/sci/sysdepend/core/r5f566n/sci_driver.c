/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	sci_driver.c
 */

#include <string.h>
#include <tk/tkernel.h>
#include <dev_sci.h>
#include "sci_config.h"
#include "iodefine.h"

typedef enum {
#ifdef USE_SCI0
 RSCI0,
#endif /* USE_SCI0 */
#ifdef USE_SCI1
 RSCI1,
#endif /* USE_SCI1 */
#ifdef USE_SCI2
 RSCI2,
#endif /* USE_SCI2 */
#ifdef USE_SCI3
 RSCI3,
#endif /* USE_SCI3 */
#ifdef USE_SCI4
 RSCI4,
#endif /* USE_SCI4 */
#ifdef USE_SCI5
 RSCI5,
#endif /* USE_SCI5 */
#ifdef USE_SCI6
 RSCI6,
#endif /* USE_SCI6 */
#ifdef USE_SCI7
 RSCI7,
#endif /* USE_SCI7 */
#ifdef USE_SCI8
 RSCI8,
#endif /* USE_SCI8 */
#ifdef USE_SCI9
 RSCI9,
#endif /* USE_SCI9 */
#ifdef USE_SCI10
 RSCI10,
#endif /* USE_SCI10 */
#ifdef USE_SCI11
 RSCI11,
#endif /* USE_SCI11 */
#ifdef USE_SCI12
 RSCI12,
#endif /* USE_SCI12 */
 OBJ_KIND_NUM } OBJ_KIND;
SCI_TBL sci_tbl[OBJ_KIND_NUM];
LOCAL UB sci_rbuf[OBJ_KIND_NUM][SCI_CFG_RBUF_SIZE];
#if !USE_IMALLOC
LOCAL INT sci_task_stack[OBJ_KIND_NUM][300/sizeof(INT)];
#endif /* USE_IMALLOC */

EXPORT void SCI_Read(T_DEVREQ *devreq, void *exinf)
{
SCI_TBL *sci = &sci_tbl[(UINT)exinf];
volatile struct st_sci0 __evenaccess *SCI = sci->sci;
	devreq->error = E_PAR;						// Set Error Code(Parameter Error)
	switch( devreq->start )  {
	case DN_RSMODE:
		if( devreq->size != sizeof( RsMode ) )			// Parameter Check
			break;
		*(RsMode*)devreq->buf = sci->rsmode;			// Write RS Mode
		devreq->asize = sizeof( RsMode );			// Set Actual Size
		devreq->error = E_OK;					// Set Error Code(Normal End)
		break;
	case 0:
		devreq->asize = 0;					// Set Actual Size
		if( ! SCI->SCR.BIT.RE )					// Check Receive Enable
			devreq->error = E_IO;				// Set Error Code(I/O Error)
		else if( ! devreq->size )				// Size is Zero ?
			devreq->error = ((UINT)sci->wt - sci->rd ) % SCI_CFG_RBUF_SIZE;
		else  {
			SCI->SCR.BIT.RIE = 0;				// Disable Receive Interrupt
			while( devreq->size != devreq->asize )  {	// Until Receive Finish
				if( sci->rd == sci->wt )
					break;
				((UB*)devreq->buf)[devreq->asize++] = sci_rbuf[(UINT)exinf][sci->rd];
				sci->rd = ( sci->rd + 1 ) % SCI_CFG_RBUF_SIZE;
			}						// Increment Read Pointer
			if( devreq->size == devreq->asize )  {
				devreq->error = E_OK;			// Set Error Code(Normal End)
				SCI->SCR.BIT.RIE = 1;			// Enable Receive Interrupt
				break;
			}
			else  {
				sci->rwreq = devreq;			// Set Receive Wait Request
				SCI->SCR.BIT.RIE = 1;			// Enable Receive Interrupt
				return;
			}
		}
		break;
	}
	tk_set_flg( sci->flgid, SCI_RCVEND );				// Set Receive End SCI Event
}

EXPORT void SCI_Write(T_DEVREQ *devreq, void *exinf)
{
SCI_TBL *sci = &sci_tbl[(UINT)exinf];
volatile struct st_sci0 __evenaccess *SCI = sci->sci;
UW baud;
INT i;
	devreq->error = E_PAR;						// Set Error Code(Parameter Error)
	switch( devreq->start )  {
	case DN_RSMODE:
		if( devreq->size != sizeof( RsMode ) )			// Parameter Check
			break;
		sci->rsmode = *(RsMode*)devreq->buf;			// Save RS Mode
		baud = sci->pclk / 32.F / sci->rsmode.baud - 1;		// Analize Bit Arte
		if( sci->rsmode.parity == 3  || !(sci->rsmode.datalen & 2)
		|| (sci->rsmode.stopbits & 1)|| baud >= 255 || baud <= 1 )
			break;						// RS Mode Check
		SCI->SCR.BYTE = 0x00;					// Disable TX and RX
		if( sci->rwreq )  {					// Exist Receive Wait Request ?
			sci->rwreq->error = E_IO;			// Set Error Code(I/O Error)
			tk_set_flg( sci->flgid, SCI_RCVEND );		// Set Receive End SCI Event
		}
		sci->rd = sci->wt = 0;					// Clear Read Write Pointer
		SCI->SMR.BYTE = 0x00;					// Asynchronous mode, CKS:n=0
		if( sci->rsmode.datalen == 2 )				// Character Length is 7 Bit ?
			SCI->SMR.BIT.CHR = 1;				// Set Character Length
		if( sci->rsmode.stopbits == 2 )				// Stop Bit Length is 2 Bit ?
			SCI->SMR.BIT.STOP = 1;				// Set Character Length
		if( sci->rsmode.parity != 0 )  {			// Parity Enable ?
			SCI->SMR.BIT.PE = 1;				// Set Parity Enable
			if( sci->rsmode.parity == 1 )				// Odd Parity ?
				SCI->SMR.BIT.PM = 1;			// Set Odd Parity
		}
		SCI->BRR = baud;					// Set Bit Rate Register
		SCI->SEMR.BYTE = 0x04;					// Enable Bit Rate Modulation
		SCI->MDDR = (baud+1) * 8192.F * sci->rsmode.baud / sci->pclk + 0.5F;
		for( i=0 ; i<ICLK/4/sci->rsmode.baud ; i++ )  ;		// Wait 1 Bit Period
		SCI->SCR.BYTE = 0x70;					// Enable TE,RE,RIE
		devreq->asize = sizeof( RsMode );			// Set Actual Size
		devreq->error = E_OK;					// Set Error Code(Normal End)
		break;
	case 0:
		devreq->asize = 0;					// Set Actual Size
		if( ! SCI->SCR.BIT.TE )					// Check Transmit Enable
			devreq->error = E_IO;				// Set Error Code(I/O Error)
		else if( ! devreq->size )				// Size is Zero ?
			devreq->error = E_OK;				// Set Error Code(Normal End)
		else  {
			devreq->asize ++;				// Increment Actual Size
			SCI->TDR = ((UB*)devreq->buf)[0];		// Set Send Data
			if( devreq->size == 1 )				// Last Data ?
				SCI->SCR.BIT.TEIE = 1;			// Enable TEIE
			else
				SCI->SCR.BIT.TIE = 1;			// Enable TIE
			return;
		}
		break;
	}
	tk_set_flg( sci->flgid, SCI_SNDEND );				// Set Send End SCI Event
}

LOCAL void SCI_RXI_Handler(UINT ch)
{
SCI_TBL *sci = &sci_tbl[ch];
volatile struct st_sci0 __evenaccess *SCI = sci->sci;
T_DEVREQ *devreq = sci->rwreq;
	if( devreq && devreq->size != devreq->asize )  {	// Exist Receive Wait Device Request
		((UB*)devreq->buf)[devreq->asize++] = SCI->RDR;	// Set Receive Data
		if( devreq->size == devreq->asize )  {		// Receive End ?
			devreq->error = E_OK;			// Set Error Code(Normal End)
			tk_set_flg( sci->flgid, SCI_RCVEND );	// Set Receive End Event
		}
	}
	else if( sci->rd != ( sci->wt + 1 ) % SCI_CFG_RBUF_SIZE )  {
		sci_rbuf[ch][sci->wt] = SCI->RDR;		// Set Receive Data
		sci->wt = ( sci->wt + 1 ) % SCI_CFG_RBUF_SIZE;	// Increment Write Pointer
	}
	else
		if( SCI->RDR )  ;				// Dummy Read
}

LOCAL void SCI_TXI_Handler(UINT ch)
{
SCI_TBL *sci = &sci_tbl[ch];
volatile struct st_sci0 __evenaccess *SCI = sci->sci;
T_DEVREQ *devreq = sci->swreq;					// Set Send Wait Device Request
	if( devreq->size == devreq->asize + 1 )  {		// Next Data is Last Data ?
		SCI->SCR.BIT.TIE = 0;				// Disable TIE
		SCI->SCR.BIT.TEIE = 1;				// Enable TEIE
	}
	SCI->TDR = ((UB*)devreq->buf)[devreq->asize++];		// Set Send Data
}

LOCAL void SCI_TEI_Handler(UINT ch)
{
SCI_TBL *sci = &sci_tbl[ch];
volatile struct st_sci0 __evenaccess *SCI = sci->sci;
	SCI->SCR.BIT.TEIE = 0;					// Disable TEIE
	sci->swreq->error = E_OK;				// Set Error Code(Normal End)
	tk_set_flg( sci->flgid, SCI_SNDEND );			// Set Send End Event
}

#ifdef USE_SCI0
LOCAL void sci0_eri0_hdr(UINT dintno)
{
	SCI0.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci0_rxi0_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI0 );				// Call SCI_RXI_Handler
}

LOCAL void sci0_txi0_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI0 );				// Call SCI_TXI_Handler
}

LOCAL void sci0_tei0_hdr(UINT dintno)
{
	SCI0.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI0 );				// Call SCI_TEI_Handler
}
#endif /* USE_SIIC0 */

#ifdef USE_SCI1
LOCAL void sci1_eri1_hdr(UINT dintno)
{
	SCI1.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci1_rxi1_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI1 );				// Call SCI_RXI_Handler
}

LOCAL void sci1_txi1_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI1 );				// Call SCI_TXI_Handler
}

LOCAL void sci1_tei1_hdr(UINT dintno)
{
	SCI1.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI1 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI1 */

#ifdef USE_SCI2
LOCAL void sci2_eri2_hdr(UINT dintno)
{
	SCI2.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci2_rxi2_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI2 );				// Call SCI_RXI_Handler
}

LOCAL void sci2_txi2_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI2 );				// Call SCI_TXI_Handler
}

LOCAL void sci2_tei2_hdr(UINT dintno)
{
	SCI2.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI2 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI2 */

#ifdef USE_SCI3
LOCAL void sci3_eri3_hdr(UINT dintno)
{
	SCI3.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci3_rxi3_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI3 );				// Call SCI_RXI_Handler
}

LOCAL void sci3_txi3_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI3 );				// Call SCI_TXI_Handler
}

LOCAL void sci3_tei3_hdr(UINT dintno)
{
	SCI3.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI3 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI3 */

#ifdef USE_SCI4
LOCAL void sci4_eri4_hdr(UINT dintno)
{
	SCI4.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci4_rxi4_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI4 );				// Call SCI_RXI_Handler
}

LOCAL void sci4_txi4_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI4 );				// Call SCI_TXI_Handler
}

LOCAL void sci4_tei4_hdr(UINT dintno)
{
	SCI4.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI4 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI4 */

#ifdef USE_SCI5
LOCAL void sci5_eri5_hdr(UINT dintno)
{
	SCI5.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci5_rxi5_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI5 );				// Call SCI_RXI_Handler
}

LOCAL void sci5_txi5_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI5 );				// Call SCI_TXI_Handler
}

LOCAL void sci5_tei5_hdr(UINT dintno)
{
	SCI5.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI5 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI5 */

#ifdef USE_SCI6
LOCAL void sci6_eri6_hdr(UINT dintno)
{
	SCI6.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci6_rxi6_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI6 );				// Call SCI_RXI_Handler
}

LOCAL void sci6_txi6_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI6 );				// Call SCI_TXI_Handler
}

LOCAL void sci6_tei6_hdr(UINT dintno)
{
	SCI6.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI6 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI6 */

#ifdef USE_SCI7
LOCAL void sci7_eri7_hdr(UINT dintno)
{
	SCI7.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci7_rxi7_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI7 );				// Call SCI_RXI_Handler
}

LOCAL void sci7_txi7_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI7 );				// Call SCI_TXI_Handler
}

LOCAL void sci7_tei7_hdr(UINT dintno)
{
	SCI7.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI7 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI7 */

#ifdef USE_SCI8
LOCAL void sci8_eri8_hdr(UINT dintno)
{
	SCI8.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci8_rxi8_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI8 );				// Call SCI_RXI_Handler
}

LOCAL void sci8_txi8_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI8 );				// Call SCI_TXI_Handler
}

LOCAL void sci8_tei8_hdr(UINT dintno)
{
	SCI8.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI8 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI8 */

#ifdef USE_SCI9
LOCAL void sci9_eri9_hdr(UINT dintno)
{
	SCI9.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci9_rxi9_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI9 );				// Call SCI_RXI_Handler
}

LOCAL void sci9_txi9_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI9 );				// Call SCI_TXI_Handler
}

LOCAL void sci9_tei9_hdr(UINT dintno)
{
	SCI9.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI9 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI9 */

#ifdef USE_SCI10
LOCAL void sci10_eri10_hdr(UINT dintno)
{
	SCI10.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci10_rxi10_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI10 );				// Call SCI_RXI_Handler
}

LOCAL void sci10_txi10_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI10 );				// Call SCI_TXI_Handler
}

LOCAL void sci10_tei10_hdr(UINT dintno)
{
	SCI10.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI10 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI10 */

#ifdef USE_SCI11
LOCAL void sci11_eri11_hdr(UINT dintno)
{
	SCI11.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci11_rxi11_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI11 );				// Call SCI_RXI_Handler
}

LOCAL void sci11_txi11_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI11 );				// Call SCI_TXI_Handler
}

LOCAL void sci11_tei11_hdr(UINT dintno)
{
	SCI11.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI11 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI11 */

#ifdef USE_SCI12
LOCAL void sci12_eri12_hdr(UINT dintno)
{
	SCI12.SSR.BYTE &= 0xC7;					// Clear Error Flag
}

LOCAL void sci12_rxi12_hdr(UINT dintno)
{
	SCI_RXI_Handler( RSCI12 );				// Call SCI_RXI_Handler
}

LOCAL void sci12_txi12_hdr(UINT dintno)
{
	SCI_TXI_Handler( RSCI12 );				// Call SCI_TXI_Handler
}

LOCAL void sci12_tei12_hdr(UINT dintno)
{
	SCI12.SCR.BIT.TEIE = 0;					// Disable TEIE Interrupt
	SCI_TEI_Handler( RSCI12 );				// Call SCI_TEI_Handler
}
#endif /* USE_SCI12 */

LOCAL ER sciCreFlg(SCI_TBL *sci, T_CFLG *p_cflg)
{
	p_cflg->flgatr = TA_TPRI | TA_WMUL;			// Set EventFlag Attribute
#if USE_OBJECT_NAME
	p_cflg->flgatr |= TA_DSNAME;				// Set EventFlag Attribute
#ifdef CLANGSPEC
	strcpy( p_cflg->dsname, "sci*_f" );			// Set Debugger Suport Name
#else
	strcpy( (char*)p_cflg->dsname, "sci*_f" );		// Set Debugger Suport Name
#endif /* CLANGSPEC */
	p_cflg->dsname[3] = sci->drvname[3];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
	p_cflg->iflgptn = 0;					// Set Initial Bit Pattern
	return sci->flgid = tk_cre_flg( p_cflg );		// Create SCI EventFlag
}

LOCAL ER sciCreTsk(SCI_TBL *sci, UINT ch, T_CTSK *p_ctsk)
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
	p_ctsk->itskpri = SCI_CFG_TASK_PRIORITY;		// Set Task Priority
#ifdef CLANGSPEC
	p_ctsk->task = sci_tsk;					// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( p_ctsk->dsname, "sci*_t" );			// Set Task Debugger Suport Name
	p_ctsk->dsname[3] = sci->drvname[3];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
#else
	p_ctsk->task = (FP)sci_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)p_ctsk->dsname, "sci*_t" );		// Set Task Debugger Suport Name
	p_ctsk->dsname[3] = siic->drvname[3];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( p_ctsk )) <= E_OK )		// Create SCI Control Task
		return objid;
	return tk_sta_tsk( objid, 0 );				// Start SCI Control Task
}

EXPORT ER sciDrvEntry(void)
{
ER ercd;
union { T_CTSK t_ctsk; T_CFLG t_cflg; T_DDEV t_ddev; T_DINT t_dint; } u;
#ifdef USE_SCI0
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI0 ) )  {					// SCI0 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI0 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI0 ) = 0;					// Enable SCI0
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD0_P21)
	MPC.P21PFS.BYTE = 0x0A;					// P21 is RXD0 Pin
#elif defined(USE_RXD0_P33)
	MPC.P33PFS.BYTE = 0x0B;					// P33 is RXD0 Pin
#endif
#if defined(USE_TXD0_P20)
	MPC.P20PFS.BYTE = 0x0A;					// P20 is TXD0 Pin
#elif defined(USE_TXD0_P32)
	MPC.P32PFS.BYTE = 0x0B;					// P32 is TXD0 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD0_P21)
	PORT2.PMR.BIT.B1 = 1;					// P21 is Peripheral Pin
#elif defined(USE_RXD0_P33)
	PORT3.PMR.BIT.B3 = 1;					// P33 is Peripheral Pin
#endif
#if defined(USE_TXD0_P20)
	PORT2.PMR.BIT.B0 = 1;					// P20 is Peripheral Pin
#elif defined(USE_TXD0_P32)
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
	EnableInt( VECT( SCI0, RXI0 ), SCI_CFG_INT_PRIORITY );	// Enable SCI0 RXI0 Interrupt
	EnableInt( VECT( SCI0, TXI0 ), SCI_CFG_INT_PRIORITY );	// Enable SCI0 TXI0 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[0] = sci0_tei0_hdr;			// Set Handler Address
	GroupBL0Table[1] = sci0_eri0_hdr;			// Set Handler Address
	EN( SCI0, TEI0 ) = 1;					// Enable SCI0 TEI0 Group Interrupt
	EN( SCI0, ERI0 ) = 1;					// Enable SCI0 ERI0 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI0].drvname = "scia";			// Set Driver Name
	sci_tbl[RSCI0].sci = (void*)&SCI0;			// Set SCI Channel Address
	sci_tbl[RSCI0].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI0].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI0], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI0], RSCI0, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI0].drvname, RSCI0, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI0 */

#ifdef USE_SCI1
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI1 ) )  {					// SCI1 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI1 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI1 ) = 0;					// Enable SCI1
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD1_P15)
	MPC.P15PFS.BYTE = 0x0A;					// P15 is RXD1 Pin
#elif defined(USE_RXD1_P30)
	MPC.P30PFS.BYTE = 0x0A;					// P30 is RXD1 Pin
#elif defined(USE_RXD1_PF2)
	MPC.PF2PFS.BYTE = 0x0A;					// PF2 is RXD1 Pin
#endif
#if defined(USE_TXD1_P16)
	MPC.P16PFS.BYTE = 0x0A;					// P16 is TXD1 Pin
#elif defined(USE_TXD1_P26)
	MPC.P26PFS.BYTE = 0x0A;					// P26 is TXD1 Pin
#elif defined(USE_TXD1_PF0)
	MPC.PF0PFS.BYTE = 0x0A;					// PF0 is TXD1 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD1_P15)
	PORT1.PMR.BIT.B5 = 1;					// P15 is Peripheral Pin
#elif defined(USE_RXD1_P30)
	PORT3.PMR.BIT.B0 = 1;					// P30 is Peripheral Pin
#elif defined(USE_RXD1_PF2)
	PORTF.PMR.BIT.B2 = 1;					// PF2 is Peripheral Pin
#endif
#if defined(USE_TXD1_P16)
	PORT1.PMR.BIT.B6 = 1;					// P16 is Peripheral Pin
#elif defined(USE_TXD1_P26)
	PORT2.PMR.BIT.B6 = 1;					// P26 is Peripheral Pin
#elif defined(USE_TXD1_PF0)
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
	EnableInt( VECT( SCI1, RXI1 ), SCI_CFG_INT_PRIORITY );	// Enable SCI1 RXI1 Interrupt
	EnableInt( VECT( SCI1, TXI1 ), SCI_CFG_INT_PRIORITY );	// Enable SCI1 TXI1 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[2] = sci1_tei1_hdr;			// Set Handler Address
	GroupBL0Table[3] = sci1_eri1_hdr;			// Set Handler Address
	EN( SCI1, TEI1 ) = 1;					// Enable SCI1 TEI1 Group Interrupt
	EN( SCI1, ERI1 ) = 1;					// Enable SCI1 ERI1 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI1].drvname = "scib";			// Set Driver Name
	sci_tbl[RSCI1].sci = (void*)&SCI1;			// Set SCI Channel Address
	sci_tbl[RSCI1].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI1].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI1], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI1], RSCI1, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI1].drvname, RSCI1, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI1 */

#ifdef USE_SCI2
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI2 ) )  {					// SCI2 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI2 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI2 ) = 0;					// Enable SCI2
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD2_P12)
	MPC.P12PFS.BYTE = 0x0A;					// P12 is RXD2 Pin
#elif defined(USE_RXD2_P52)
	MPC.P52PFS.BYTE = 0x0A;					// P52 is RXD2 Pin
#endif
#if defined(USE_TXD2_P13)
	MPC.P13PFS.BYTE = 0x0A;					// P13 is TXD2 Pin
#elif defined(USE_TXD2_P50)
	MPC.P50PFS.BYTE = 0x0A;					// P50 is TXD2 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD2_P12)
	PORT1.PMR.BIT.B2 = 1;					// P12 is Peripheral Pin
#elif defined(USE_RXD2_P52)
	PORT5.PMR.BIT.B2 = 1;					// P50 is Peripheral Pin
#endif
#if defined(USE_TXD2_P13)
	PORT1.PMR.BIT.B3 = 1;					// P13 is Peripheral Pin
#elif defined(USE_TXD2_P50)
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
	EnableInt( VECT( SCI2, RXI2 ), SCI_CFG_INT_PRIORITY );	// Enable SCI2 RXI2 Interrupt
	EnableInt( VECT( SCI2, TXI2 ), SCI_CFG_INT_PRIORITY );	// Enable SCI2 TXI2 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[4] = sci2_tei2_hdr;			// Set Handler Address
	GroupBL0Table[5] = sci2_eri2_hdr;			// Set Handler Address
	EN( SCI2, TEI2 ) = 1;					// Enable SCI2 TEI2 Group Interrupt
	EN( SCI2, ERI2 ) = 1;					// Enable SCI2 ERI2 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI2].drvname = "scic";			// Set Driver Name
	sci_tbl[RSCI2].sci = (void*)&SCI2;			// Set SCI Channel Address
	sci_tbl[RSCI2].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI2].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI2], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI2], RSCI2, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI2].drvname, RSCI2, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI2 */

#ifdef USE_SCI3
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI3 ) )  {					// SCI3 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI3 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI3 ) = 0;					// Enable SCI3
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD3_P16)
	MPC.P16PFS.BYTE = 0x0B;					// P16 is RXD3 Pin
#elif defined(USE_RXD3_P25)
	MPC.P25PFS.BYTE = 0x0A;					// P25 is RXD3 Pin
#endif
#if defined(USE_TXD3_P17)
	MPC.P17PFS.BYTE = 0x0B;					// P17 is TXD3 Pin
#elif defined(USE_TXD3_P23)
	MPC.P23PFS.BYTE = 0x0A;					// P23 is TXD3 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD3_P16)
	PORT1.PMR.BIT.B6 = 1;					// P16 is Peripheral Pin
#elif defined(USE_RXD3_P25)
	PORT2.PMR.BIT.B5 = 1;					// P25 is Peripheral Pin
#endif
#if defined(USE_TXD3_P17)
	PORT1.PMR.BIT.B7 = 1;					// P17 is Peripheral Pin
#elif defined(USE_TXD3_P23)
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
	EnableInt( VECT( SCI3, RXI3 ), SCI_CFG_INT_PRIORITY );	// Enable SCI3 RXI3 Interrupt
	EnableInt( VECT( SCI3, TXI3 ), SCI_CFG_INT_PRIORITY );	// Enable SCI3 TXI3 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[6] = sci3_tei3_hdr;			// Set Handler Address
	GroupBL0Table[7] = sci3_eri3_hdr;			// Set Handler Address
	EN( SCI3, TEI3 ) = 1;					// Enable SCI3 TEI3 Group Interrupt
	EN( SCI3, ERI3 ) = 1;					// Enable SCI3 ERI3 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI3].drvname = "scid";			// Set Driver Name
	sci_tbl[RSCI3].sci = (void*)&SCI3;			// Set SCI Channel Address
	sci_tbl[RSCI3].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI3].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI3], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI3], RSCI3, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI3].drvname, RSCI3, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI3 */

#ifdef USE_SCI4
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI4 ) )  {					// SCI4 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI4 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI4 ) = 0;					// Enable SCI4
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD4_PB0)
	MPC.PB0PFS.BYTE = 0x0A;					// PB0 is RXD4 Pin
#endif
#if defined(USE_TXD4_PB1)
	MPC.PB1PFS.BYTE = 0x0A;					// PB1 is TXD4 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD4_PB0)
	PORTB.PMR.BIT.B0 = 1;					// PB0 is Peripheral Pin
#endif
#if defined(USE_TXD4_PB1)
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
	EnableInt( VECT( SCI4, RXI4 ), SCI_CFG_INT_PRIORITY );	// Enable SCI4 RXI4 Interrupt
	EnableInt( VECT( SCI4, TXI4 ), SCI_CFG_INT_PRIORITY );	// Enable SCI4 TXI4 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[8] = sci4_tei4_hdr;			// Set Handler Address
	GroupBL0Table[9] = sci4_eri4_hdr;			// Set Handler Address
	EN( SCI4, TEI4 ) = 1;					// Enable SCI4 TEI4 Group Interrupt
	EN( SCI4, ERI4 ) = 1;					// Enable SCI4 ERI4 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI4].drvname = "scie";			// Set Driver Name
	sci_tbl[RSCI4].sci = (void*)&SCI4;			// Set SCI Channel Address
	sci_tbl[RSCI4].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI4].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI4], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI4], RSCI4, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI4].drvname, RSCI4, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI4 */

#ifdef USE_SCI5
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI5 ) )  {					// SCI5 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI5 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI5 ) = 0;					// Enable SCI5
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD5_PA2)
	MPC.PA2PFS.BYTE = 0x0A;					// PA2 is RXD5 Pin
#elif defined(USE_RXD5_PA3)
	MPC.PA3PFS.BYTE = 0x0A;					// PA3 is RXD5 Pin
#elif defined(USE_RXD5_PC2)
	MPC.PC2PFS.BYTE = 0x0A;					// PC2 is RXD5 Pin
#endif
#if defined(USE_TXD5_PA4)
	MPC.PA4PFS.BYTE = 0x0A;					// PA4 is TXD5 Pin
#elif defined(USE_TXD5_PC3)
	MPC.PC3PFS.BYTE = 0x0A;					// PC3 is TXD5 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD5_PA2)
	PORTA.PMR.BIT.B2 = 1;					// PA2 is Peripheral Pin
#elif defined(USE_RXD5_PA3)
	PORTA.PMR.BIT.B3 = 1;					// PA3 is Peripheral Pin
#elif defined(USE_RXD5_PC2)
	PORTC.PMR.BIT.B2 = 1;					// PC2 is Peripheral Pin
#endif
#if defined(USE_TXD5_PA4)
	PORTA.PMR.BIT.B4 = 1;					// PA4 is Peripheral Pin
#elif defined(USE_TXD5_PC3)
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
	EnableInt( VECT( SCI5, RXI5 ), SCI_CFG_INT_PRIORITY );	// Enable SCI5 RXI5 Interrupt
	EnableInt( VECT( SCI5, TXI5 ), SCI_CFG_INT_PRIORITY );	// Enable SCI5 TXI5 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[10] = sci5_tei5_hdr;			// Set Handler Address
	GroupBL0Table[11] = sci5_eri5_hdr;			// Set Handler Address
	EN( SCI5, TEI5 ) = 1;					// Enable SCI5 TEI5 Group Interrupt
	EN( SCI5, ERI5 ) = 1;					// Enable SCI5 ERI5 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI5].drvname = "scif";			// Set Driver Name
	sci_tbl[RSCI5].sci = (void*)&SCI5;			// Set SCI Channel Address
	sci_tbl[RSCI5].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI5].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI5], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI5], RSCI5, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI5].drvname, RSCI5, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI5 */

#ifdef USE_SCI6
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI6 ) )  {					// SCI6 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI6 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI6 ) = 0;					// Enable SCI6
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD6_P01)
	MPC.P01PFS.BYTE = 0x0A;					// P01 is RXD6 Pin
#elif defined(USE_RXD6_P33)
	MPC.P33PFS.BYTE = 0x0A;					// P33 is RXD6 Pin
#elif defined(USE_RXD6_PB0)
	MPC.PB0PFS.BYTE = 0x0B;					// PB0 is RXD6 Pin
#endif
#if defined(USE_TXD6_P00)
	MPC.P00PFS.BYTE = 0x0A;					// P00 is TXD6 Pin
#elif defined(USE_TXD6_P32)
	MPC.P32PFS.BYTE = 0x0A;					// P32 is TXD6 Pin
#elif defined(USE_TXD6_PB1)
	MPC.PB1PFS.BYTE = 0x0B;					// PB1 is TXD6 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD6_P01)
	PORT0.PMR.BIT.B1 = 1;					// P01 is Peripheral Pin
#elif defined(USE_RXD6_P33)
	PORT3.PMR.BIT.B3 = 1;					// P33 is Peripheral Pin
#elif defined(USE_RXD6_PB0)
	PORTB.PMR.BIT.B0 = 1;					// PB0 is Peripheral Pin
#endif
#if defined(USE_TXD6_P00)
	PORT0.PMR.BIT.B0 = 1;					// P00 is Peripheral Pin
#elif defined(USE_TXD6_P32)
	PORT3.PMR.BIT.B2 = 1;					// P32 is Peripheral Pin
#elif defined(USE_TXD6_PB1)
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
	EnableInt( VECT( SCI6, RXI6 ), SCI_CFG_INT_PRIORITY );	// Enable SCI6 RXI6 Interrupt
	EnableInt( VECT( SCI6, TXI6 ), SCI_CFG_INT_PRIORITY );	// Enable SCI6 TXI6 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[12] = sci6_tei6_hdr;			// Set Handler Address
	GroupBL0Table[13] = sci6_eri6_hdr;			// Set Handler Address
	EN( SCI6, TEI6 ) = 1;					// Enable SCI6 TEI6 Group Interrupt
	EN( SCI6, ERI6 ) = 1;					// Enable SCI6 ERI6 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI6].drvname = "scig";			// Set Driver Name
	sci_tbl[RSCI6].sci = (void*)&SCI6;			// Set SCI Channel Address
	sci_tbl[RSCI6].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI6].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI6], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI6], RSCI6, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI6].drvname, RSCI6, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI6 */

#ifdef USE_SCI7
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI7 ) )  {					// SCI7 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI7 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI7 ) = 0;					// Enable SCI7
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD7_P57)
	MPC.P57PFS.BYTE = 0x0A;					// P57 is RXD7 Pin
#elif defined(USE_RXD7_P92)
	MPC.P92PFS.BYTE = 0x0A;					// P92 is RXD7 Pin
#elif defined(USE_RXD7_PH1)
	MPC.PH1PFS.BYTE = 0x0A;					// PH1 is RXD7 Pin
#endif
#if defined(USE_TXD7_P55)
	MPC.P55PFS.BYTE = 0x0A;					// P55 is TXD7 Pin
#elif defined(USE_TXD7_P90)
	MPC.P90PFS.BYTE = 0x0A;					// P90 is TXD7 Pin
#elif defined(USE_TXD7_PH2)
	MPC.PH2PFS.BYTE = 0x0A;					// PH2 is TXD7 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD7_P57)
	PORT5.PMR.BIT.B7 = 1;					// P57 is Peripheral Pin
#elif defined(USE_RXD7_P92)
	PORT9.PMR.BIT.B2 = 1;					// P92 is Peripheral Pin
#elif defined(USE_RXD7_PH1)
	PORTH.PMR.BIT.B1 = 1;					// PH1 is Peripheral Pin
#endif
#if defined(USE_TXD7_P55)
	PORT5.PMR.BIT.B5 = 1;					// P55 is Peripheral Pin
#elif defined(USE_TXD7_P90)
	PORT9.PMR.BIT.B0 = 1;					// P90 is Peripheral Pin
#elif defined(USE_TXD7_PH2)
	PORTH.PMR.BIT.B2 = 1;					// PH2 is Peripheral Pin
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
	EnableInt( VECT( SCI7, RXI7 ), SCI_CFG_INT_PRIORITY );	// Enable SCI7 RXI7 Interrupt
	EnableInt( VECT( SCI7, TXI7 ), SCI_CFG_INT_PRIORITY );	// Enable SCI7 TXI7 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[14] = sci7_tei7_hdr;			// Set Handler Address
	GroupBL0Table[15] = sci7_eri7_hdr;			// Set Handler Address
	EN( SCI7, TEI7 ) = 1;					// Enable SCI7 TEI7 Group Interrupt
	EN( SCI7, ERI7 ) = 1;					// Enable SCI7 ERI7 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI7].drvname = "scih";			// Set Driver Name
	sci_tbl[RSCI7].sci = (void*)&SCI6;			// Set SCI Channel Address
	sci_tbl[RSCI7].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI7].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI7], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI7], RSCI7, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI7].drvname, RSCI7, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI7 */

#ifdef USE_SCI8
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI8 ) )  {					// SCI8 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI8 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI8 ) = 0;					// Enable SCI8
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD8_PC6)
	MPC.PC6PFS.BYTE = 0x0A;					// PC6 is RXD8 Pin
#elif defined(USE_RXD8_PJ1)
	MPC.PJ1PFS.BYTE = 0x0A;					// PJ1 is RXD8 Pin
#elif defined(USE_RXD8_PK1)
	MPC.PK1PFS.BYTE = 0x0A;					// PK1 is RXD8 Pin
#endif
#if defined(USE_TXD8_PC7)
	MPC.PC7PFS.BYTE = 0x0A;					// PC7 is TXD8 Pin
#elif defined(USE_TXD8_PJ2)
	MPC.PJ2PFS.BYTE = 0x0A;					// PJ2 is TXD8 Pin
#elif defined(USE_TXD8_PK2)
	MPC.PK2PFS.BYTE = 0x0A;					// PK2 is TXD8 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD8_PC6)
	PORTC.PMR.BIT.B6 = 1;					// PC6 is Peripheral Pin
#elif defined(USE_RXD8_PJ1)
	PORTJ.PMR.BIT.B1 = 1;					// PJ1 is Peripheral Pin
#elif defined(USE_RXD8_PK1)
	PORTK.PMR.BIT.B1 = 1;					// PK1 is Peripheral Pin
#endif
#if defined(USE_TXD8_PC7)
	PORTC.PMR.BIT.B7 = 1;					// PC7 is Peripheral Pin
#elif defined(USE_TXD8_PJ2)
	PORTJ.PMR.BIT.B2 = 1;					// PJ2 is Peripheral Pin
#elif defined(USE_TXD8_PK2)
	PORTK.PMR.BIT.B2 = 1;					// PK2 is Peripheral Pin
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
	EnableInt( VECT( SCI8, RXI8 ), SCI_CFG_INT_PRIORITY );	// Enable SCI8 RXI8 Interrupt
	EnableInt( VECT( SCI8, TXI8 ), SCI_CFG_INT_PRIORITY );	// Enable SCI8 TXI8 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL1 ) )  {				// BL1 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL1Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL1Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL1 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL1 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL1 Interrupt
	GroupBL1Table[24] = sci8_tei8_hdr;			// Set Handler Address
	GroupBL1Table[25] = sci8_eri8_hdr;			// Set Handler Address
	EN( SCI8, TEI8 ) = 1;					// Enable SCI8 TEI8 Group Interrupt
	EN( SCI8, ERI8 ) = 1;					// Enable SCI8 ERI8 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL1 ) != SCI_CFG_INT_PRIORITY )	// BL1 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI8].drvname = "scii";			// Set Driver Name
	sci_tbl[RSCI8].sci = (void*)&SCI8;			// Set SCI Channel Address
	sci_tbl[RSCI8].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI8].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI8], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI8], RSCI8, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI8].drvname, RSCI8, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI8 */

#ifdef USE_SCI9
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI9 ) )  {					// SCI9 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI9 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI9 ) = 0;					// Enable SCI9
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD9_PB6)
	MPC.PB6PFS.BYTE = 0x0A;					// PB6 is RXD9 Pin
#elif defined(USE_RXD9_PL1)
	MPC.PL1PFS.BYTE = 0x0A;					// PL1 is RXD9 Pin
#endif
#if defined(USE_TXD9_PB7)
	MPC.PB7PFS.BYTE = 0x0A;					// PB7 is TXD9 Pin
#elif defined(USE_TXD9_PL2)
	MPC.PL2PFS.BYTE = 0x0A;					// PL2 is TXD9 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD9_PB6)
	PORTB.PMR.BIT.B6 = 1;					// PB6 is Peripheral Pin
#elif defined(USE_RXD9_PL1)
	PORTL.PMR.BIT.B1 = 1;					// PL1 is Peripheral Pin
#endif
#if defined(USE_TXD9_PB7)
	PORTB.PMR.BIT.B7 = 1;					// PB7 is Peripheral Pin
#elif defined(USE_TXD9_PL2)
	PORTL.PMR.BIT.B2 = 1;					// PL2 is Peripheral Pin
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
	EnableInt( VECT( SCI9, RXI9 ), SCI_CFG_INT_PRIORITY );	// Enable SCI9 RXI9 Interrupt
	EnableInt( VECT( SCI9, TXI9 ), SCI_CFG_INT_PRIORITY );	// Enable SCI9 TXI9 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL1 ) )  {				// BL1 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL1Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL1Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL1 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL1 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL1 Interrupt
	GroupBL1Table[26] = sci9_tei9_hdr;			// Set Handler Address
	GroupBL1Table[27] = sci9_eri9_hdr;			// Set Handler Address
	EN( SCI9, TEI9 ) = 1;					// Enable SCI9 TEI9 Group Interrupt
	EN( SCI9, ERI9 ) = 1;					// Enable SCI9 ERI9 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL1 ) != SCI_CFG_INT_PRIORITY )	// BL1 Group IPR != IIC IPR ?
		return E_IO;
	sci_tbl[RSCI9].drvname = "scij";			// Set Driver Name
	sci_tbl[RSCI9].sci = (void*)&SCI9;			// Set SCI Channel Address
	sci_tbl[RSCI9].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI9].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI9], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI9], RSCI9, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI9].drvname, RSCI9, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI9 */

#ifdef USE_SCI10
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI10 ) )  {				// SCI10 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI10 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI10 ) = 0;					// Enable SCI10
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD10_P81)
	MPC.P81PFS.BYTE = 0x0A;					// P81 is RXD10 Pin
#elif defined(USE_RXD10_P86)
	MPC.P86PFS.BYTE = 0x0A;					// P86 is RXD10 Pin
#elif defined(USE_RXD10_PC6)
	MPC.PC6PFS.BYTE = 0x24;					// PC6 is RXD10 Pin
#elif defined(USE_RXD10_PM1)
	MPC.PM1PFS.BYTE = 0x0A;					// PM1 is RXD10 Pin
#endif
#if defined(USE_TXD10_P82)
	MPC.P82PFS.BYTE = 0x0A;					// P82 is TXD10 Pin
#elif defined(USE_TXD10_P87)
	MPC.P87PFS.BYTE = 0x0A;					// P87 is TXD10 Pin
#elif defined(USE_TXD10_PC7)
	MPC.PC7PFS.BYTE = 0x24;					// PC7 is TXD10 Pin
#elif defined(USE_TXD10_PM2)
	MPC.PM2PFS.BYTE = 0x0A;					// PM2 is TXD10 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD10_P81)
	PORT8.PMR.BIT.B1 = 1;					// P81 is Peripheral Pin
#elif defined(USE_RXD10_P86)
	PORT8.PMR.BIT.B6 = 1;					// P86 is Peripheral Pin
#elif defined(USE_RXD10_PC6)
	PORTC.PMR.BIT.B6 = 1;					// PC6 is Peripheral Pin
#elif defined(USE_RXD10_PM1)
	PORTM.PMR.BIT.B1 = 1;					// PM1 is Peripheral Pin
#endif
#if defined(USE_TXD10_P82)
	PORT8.PMR.BIT.B2 = 1;					// P81 is Peripheral Pin
#elif defined(USE_TXD10_P87)
	PORT8.PMR.BIT.B7 = 1;					// P87 is Peripheral Pin
#elif defined(USE_TXD10_PC7)
	PORTC.PMR.BIT.B7 = 1;					// PC7 is Peripheral Pin
#elif defined(USE_TXD10_PM2)
	PORTM.PMR.BIT.B2 = 1;					// PM2 is Peripheral Pin
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
	EnableInt( VECT(SCI10, RXI10), SCI_CFG_INT_PRIORITY );	// Enable SCI10 RXI10 Interrupt
	EnableInt( VECT(SCI10, TXI10), SCI_CFG_INT_PRIORITY );	// Enable SCI10 TXI10 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPAL0 ) )  {				// AL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupAL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupAL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPAL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPAL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable AL0 Interrupt
	GroupAL0Table[8] = sci10_tei10_hdr;			// Set Handler Address
	GroupAL0Table[9] = sci10_eri10_hdr;			// Set Handler Address
	EN( SCI10, TEI10 ) = 1;					// Enable SCI10 TEI10 Group Interrupt
	EN( SCI10, ERI10 ) = 1;					// Enable SCI10 ERI10 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPAL0 ) != SCI_CFG_INT_PRIORITY )	// AL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI10].drvname = "scik";			// Set Driver Name
	sci_tbl[RSCI10].sci = (void*)&SCI10;			// Set SCI Channel Address
	sci_tbl[RSCI10].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI10].pclk = PCLKA;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI10], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI10], RSCI10, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI10].drvname, RSCI10, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI10 */

#ifdef USE_SCI11
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI11 ) )  {				// SCI11 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI11 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI11 ) = 0;					// Enable SCI11
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD11_P76)
	MPC.P76PFS.BYTE = 0x0A;					// P76 is RXD11 Pin
#elif defined(USE_RXD11_PB6)
	MPC.PB6PFS.BYTE = 0x24;					// PB6 is RXD11 Pin
#elif defined(USE_RXD11_PQ1)
	MPC.PQ1PFS.BYTE = 0x0A;					// PQ1 is RXD11 Pin
#endif
#if defined(USE_TXD11_P77)
	MPC.P77PFS.BYTE = 0x0A;					// P77 is TXD11 Pin
#elif defined(USE_TXD11_PB7)
	MPC.PB7PFS.BYTE = 0x24;					// PB7 is TXD11 Pin
#elif defined(USE_TXD11_PQ2)
	MPC.PQ2PFS.BYTE = 0x0A;					// PQ2 is TXD11 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD11_P76)
	PORT7.PMR.BIT.B6 = 1;					// P76 is Peripheral Pin
#elif defined(USE_RXD11_PB6)
	PORTB.PMR.BIT.B6 = 1;					// PB6 is Peripheral Pin
#elif defined(USE_RXD11_PQ1)
	PORTQ.PMR.BIT.B1 = 1;					// PQ1 is Peripheral Pin
#endif
#if defined(USE_TXD11_P77)
	PORT7.PMR.BIT.B7 = 1;					// P77 is Peripheral Pin
#elif defined(USE_TXD11_PB7)
	PORTB.PMR.BIT.B7 = 1;					// PB7 is Peripheral Pin
#elif defined(USE_TXD11_PQ2)
	PORTQ.PMR.BIT.B2 = 1;					// PQ2 is Peripheral Pin
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
	EnableInt( VECT(SCI11, RXI11), SCI_CFG_INT_PRIORITY );	// Enable SCI11 RXI11 Interrupt
	EnableInt( VECT(SCI11, TXI11), SCI_CFG_INT_PRIORITY );	// Enable SCI11 TXI11 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPAL0 ) )  {				// AL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupAL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupAL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPAL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPAL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable AL0 Interrupt
	GroupAL0Table[12] = sci11_tei11_hdr;			// Set Handler Address
	GroupAL0Table[13] = sci11_eri11_hdr;			// Set Handler Address
	EN( SCI11, TEI11 ) = 1;					// Enable SCI11 TEI11 Group Interrupt
	EN( SCI11, ERI11 ) = 1;					// Enable SCI11 ERI11 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPAL0 ) != SCI_CFG_INT_PRIORITY )	// AL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI11].drvname = "scil";			// Set Driver Name
	sci_tbl[RSCI11].sci = (void*)&SCI11;			// Set SCI Channel Address
	sci_tbl[RSCI11].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI11].pclk = PCLKA;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI11], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI11], RSCI11, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI11].drvname, RSCI11, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI11 */

#ifdef USE_SCI12
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SCI12 ) )  {				// SCI12 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SCI12 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SCI12 ) = 0;					// Enable SCI12
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
#if defined(USE_RXD12_PE2)
	MPC.PE2PFS.BYTE = 0x0C;					// PE2 is RXD12 Pin
#endif
#if defined(USE_TXD12_PE1)
	MPC.PE1PFS.BYTE = 0x0C;					// PE1 is TXD12 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD12_PE2)
	PORTE.PMR.BIT.B2 = 1;					// PE2 is Peripheral Pin
#endif
#if defined(USE_TXD12_PE1)
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
	EnableInt( VECT(SCI12, RXI12), SCI_CFG_INT_PRIORITY );	// Enable SCI12 RXI12 Interrupt
	EnableInt( VECT(SCI12, TXI12), SCI_CFG_INT_PRIORITY );	// Enable SCI12 TXI12 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL0 ) )  {				// BL0 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL0Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL0Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL0 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL0 ), SCI_CFG_INT_PRIORITY );
	}							// Enable BL0 Interrupt
	GroupBL0Table[16] = sci12_tei12_hdr;			// Set Handler Address
	GroupBL0Table[17] = sci12_eri12_hdr;			// Set Handler Address
	EN( SCI12, TEI12 ) = 1;					// Enable SCI12 TEI12 Group Interrupt
	EN( SCI12, ERI12 ) = 1;					// Enable SCI12 ERI12 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL0 ) != SCI_CFG_INT_PRIORITY )	// BL0 Group IPR != SCI IPR ?
		return E_IO;
	sci_tbl[RSCI12].drvname = "scim";			// Set Driver Name
	sci_tbl[RSCI12].sci = (void*)&SCI12;			// Set SCI Channel Address
	sci_tbl[RSCI12].next = SCI_MAXIMUM;			// Set Next Command Pointer
	sci_tbl[RSCI12].pclk = PCLKB;				// Set Peripheral Clock
	if( ( ercd = sciCreFlg( &sci_tbl[RSCI12], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create SCI EventFlag
	if( ( ercd = sciCreTsk( &sci_tbl[RSCI12], RSCI12, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create SCI Task
	if( ( ercd = sciDriverEntry( sci_tbl[RSCI12].drvname, RSCI12, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_SCI12 */
	return E_OK;
ERROR:
	return ercd;
}