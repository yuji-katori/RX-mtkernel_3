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
#ifdef USE_SCI5
 RSCI5,
#endif /* USE_SCI5 */
#ifdef USE_SCI6
 RSCI6,
#endif /* USE_SCI6 */
#ifdef USE_SCI8
 RSCI8,
#endif /* USE_SCI8 */
#ifdef USE_SCI9
 RSCI9,
#endif /* USE_SCI9 */
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
#endif
#if defined(USE_TXD0_P20)
	MPC.P20PFS.BYTE = 0x0A;					// P20 is TXD0 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD0_P21)
	PORT2.PMR.BIT.B1 = 1;					// P21 is Peripheral Pin
#endif
#if defined(USE_TXD0_P20)
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci0_eri0_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci0_eri0_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI0, ERI0 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI0, RXI0 ), SCI_CFG_INT_PRIORITY );	// Enable SCI0 RXI0 Interrupt
	EnableInt( VECT( SCI0, TXI0 ), SCI_CFG_INT_PRIORITY );	// Enable SCI0 TXI0 Interrupt
	EnableInt( VECT( SCI0, TEI0 ), SCI_CFG_INT_PRIORITY );	// Enable SCI0 TEI0 Interrupt
	EnableInt( VECT( SCI0, ERI0 ), SCI_CFG_INT_PRIORITY );	// Enable SCI0 ERI0 Interrupt
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
#endif
#if defined(USE_TXD1_P16)
	MPC.P16PFS.BYTE = 0x0A;					// P16 is TXD1 Pin
#elif defined(USE_TXD1_P26)
	MPC.P26PFS.BYTE = 0x0A;					// P26 is TXD1 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD1_P15)
	PORT1.PMR.BIT.B5 = 1;					// P15 is Peripheral Pin
#elif defined(USE_RXD1_P30)
	PORT3.PMR.BIT.B0 = 1;					// P30 is Peripheral Pin
#endif
#if defined(USE_TXD1_P16)
	PORT1.PMR.BIT.B6 = 1;					// P16 is Peripheral Pin
#elif defined(USE_TXD1_P26)
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci1_eri1_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci1_eri1_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI1, ERI1 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI1, RXI1 ), SCI_CFG_INT_PRIORITY );	// Enable SCI1 RXI1 Interrupt
	EnableInt( VECT( SCI1, TXI1 ), SCI_CFG_INT_PRIORITY );	// Enable SCI1 TXI1 Interrupt
	EnableInt( VECT( SCI1, TEI1 ), SCI_CFG_INT_PRIORITY );	// Enable SCI1 TEI1 Interrupt
	EnableInt( VECT( SCI1, ERI1 ), SCI_CFG_INT_PRIORITY );	// Enable SCI1 ERI1 Interrupt
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci5_tei5_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci5_tei5_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI5, TEI5 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci5_eri5_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci5_eri5_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI5, ERI5 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI5, RXI5 ), SCI_CFG_INT_PRIORITY );	// Enable SCI5 RXI5 Interrupt
	EnableInt( VECT( SCI5, TXI5 ), SCI_CFG_INT_PRIORITY );	// Enable SCI5 TXI5 Interrupt
	EnableInt( VECT( SCI5, TEI5 ), SCI_CFG_INT_PRIORITY );	// Enable SCI5 TEI5 Interrupt
	EnableInt( VECT( SCI5, ERI5 ), SCI_CFG_INT_PRIORITY );	// Enable SCI5 ERI5 Interrupt
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
#if defined(USE_RXD6_P33)
	MPC.P33PFS.BYTE = 0x0B;					// P33 is RXD6 Pin
#elif defined(USE_RXD6_PB0)
	MPC.PB0PFS.BYTE = 0x0B;					// PB0 is RXD6 Pin
#endif
#if defined(USE_TXD6_P32)
	MPC.P32PFS.BYTE = 0x0B;					// P32 is TXD6 Pin
#elif defined(USE_TXD6_PB1)
	MPC.PB1PFS.BYTE = 0x0B;					// PB1 is TXD6 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD6_P33)
	PORT3.PMR.BIT.B3 = 1;					// P33 is Peripheral Pin
#elif defined(USE_RXD6_PB0)
	PORTB.PMR.BIT.B0 = 1;					// PB0 is Peripheral Pin
#endif
#if defined(USE_TXD6_P32)
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci6_tei6_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci6_tei6_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI6, TEI6 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci6_eri6_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci6_eri6_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI6, ERI6 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI6, RXI6 ), SCI_CFG_INT_PRIORITY );	// Enable SCI6 RXI6 Interrupt
	EnableInt( VECT( SCI6, TXI6 ), SCI_CFG_INT_PRIORITY );	// Enable SCI6 TXI6 Interrupt
	EnableInt( VECT( SCI6, TEI6 ), SCI_CFG_INT_PRIORITY );	// Enable SCI6 TEI6 Interrupt
	EnableInt( VECT( SCI6, ERI6 ), SCI_CFG_INT_PRIORITY );	// Enable SCI6 ERI6 Interrupt
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
#endif
#if defined(USE_TXD8_PC7)
	MPC.PC7PFS.BYTE = 0x0A;					// PC7 is TXD8 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD8_PC6)
	PORTC.PMR.BIT.B6 = 1;					// PC6 is Peripheral Pin
#endif
#if defined(USE_TXD8_PC7)
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci8_eri8_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci8_eri8_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI8, ERI8 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI8, RXI8 ), SCI_CFG_INT_PRIORITY );	// Enable SCI8 RXI8 Interrupt
	EnableInt( VECT( SCI8, TXI8 ), SCI_CFG_INT_PRIORITY );	// Enable SCI8 TXI8 Interrupt
	EnableInt( VECT( SCI8, TEI8 ), SCI_CFG_INT_PRIORITY );	// Enable SCI8 TEI8 Interrupt
	EnableInt( VECT( SCI8, ERI8 ), SCI_CFG_INT_PRIORITY );	// Enable SCI8 ERI8 Interrupt
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
#endif
#if defined(USE_TXD9_PB7)
	MPC.PB7PFS.BYTE = 0x0A;					// PB7 is TXD9 Pin
#endif
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	tk_ena_dsp( );						// Dispatch Enable
#if defined(USE_RXD9_PB6)
	PORTB.PMR.BIT.B6 = 1;					// PB6 is Peripheral Pin
#endif
#if defined(USE_TXD9_PB7)
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci9_eri9_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci9_eri9_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI9, ERI9 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( SCI9, RXI9 ), SCI_CFG_INT_PRIORITY );	// Enable SCI9 RXI9 Interrupt
	EnableInt( VECT( SCI9, TXI9 ), SCI_CFG_INT_PRIORITY );	// Enable SCI9 TXI9 Interrupt
	EnableInt( VECT( SCI9, TEI9 ), SCI_CFG_INT_PRIORITY );	// Enable SCI9 TEI9 Interrupt
	EnableInt( VECT( SCI9, ERI9 ), SCI_CFG_INT_PRIORITY );	// Enable SCI9 ERI9 Interrupt
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
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci12_tei12_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci12_tei12_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI12, TEI12 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = sci12_eri12_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)sci12_eri12_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( SCI12, ERI12 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT(SCI12, RXI12), SCI_CFG_INT_PRIORITY );	// Enable SCI12 RXI12 Interrupt
	EnableInt( VECT(SCI12, TXI12), SCI_CFG_INT_PRIORITY );	// Enable SCI12 TXI12 Interrupt
	EnableInt( VECT(SCI12, TEI12), SCI_CFG_INT_PRIORITY );	// Enable SCI12 TEI12 Interrupt
	EnableInt( VECT(SCI12, ERI12), SCI_CFG_INT_PRIORITY );	// Enable SCI12 ERI12 Interrupt
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