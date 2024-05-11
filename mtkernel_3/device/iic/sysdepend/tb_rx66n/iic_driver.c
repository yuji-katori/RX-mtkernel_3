/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	iic_driver.c
 */

#include <string.h>
#include <tk/tkernel.h>
#include <dev_iic.h>
#include "iic_config.h"
#include "iodefine.h"

typedef enum {
#ifdef USE_IIC0
 IIC0,
#endif /* USE_IIC0 */
#ifdef USE_IIC1
 IIC1,
#endif /* USE_IIC1 */
#ifdef USE_IIC2
 IIC2,
#endif /* USE_IIC2 */
 OBJ_KIND_NUM } OBJ_KIND;
IIC_TBL iic_tbl[OBJ_KIND_NUM];
#if !USE_IMALLOC
LOCAL INT iic_task_stack[OBJ_KIND_NUM][300/sizeof(INT)];
#endif /* USE_IMALLOC */

EXPORT ER IIC_Read(T_DEVREQ *devreq, void *exinf)
{
IIC_TBL *iic = &iic_tbl[(UINT)exinf];
volatile struct st_riic __evenaccess *IIC = iic->iic;
UINT flgptn;
ER ercd;
SZ i;
	if( IIC->ICCR2.BIT.BBSY )				// Check I2C Bus Occupation
		return E_IO;					// I/O Error
	IIC->ICIER.BIT.TIE = 1;					// Enable TIE
	IIC->ICCR2.BIT.ST = 1;					// Generate Start Condition
	tk_wai_flg( iic->flgid, IIC_TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	IIC->ICIER.BIT.TIE = 0;					// Disable TIE
	IIC->ICIER.BIT.RIE = 1;					// Enable RIE
	IIC->ICDRT = devreq->start << 1 | 0x01;			// Set Slave Address + R
	tk_wai_flg( iic->flgid, IIC_RXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	if( IIC->ICDRR )  ;					// Dummy Read,	NACK Receive ?
	if( ! tk_wai_flg( iic->flgid, IIC_EEI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_POL ) )  {
		IIC->ICCR2.BIT.SP = 1;				// Generate STOP Condition
		ercd = E_IO;					// I/O Error
	}
	else  {
		for( i=0 ; i<devreq->size-1 ; i++ )  {		// Data Length Loop
			tk_wai_flg( iic->flgid, IIC_RXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
			((UB*)devreq->buf)[i] = IIC->ICDRR;	// Read Receive Data
		}
		IIC->ICMR3.BIT.ACKBT = 1;			// NACK Send
		tk_wai_flg( iic->flgid, IIC_RXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		IIC->ICCR2.BIT.SP = 1;				// Generate STOP Condition
		((UB*)devreq->buf)[i] = IIC->ICDRR;		// Read Receive Data
		ercd = E_OK;					// Nomal End
	}
	IIC->ICIER.BIT.RIE = 0;					// Disable RIE
	tk_wai_flg( iic->flgid, IIC_EEI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	IIC->ICMR3.BIT.ACKBT = 0;
	return ercd;
}

EXPORT ER IIC_Write(T_DEVREQ *devreq, void *exinf)
{
IIC_TBL *iic = &iic_tbl[(UINT)exinf];
volatile struct st_riic __evenaccess *IIC = iic->iic;
UINT flgptn;
ER ercd;
SZ i;
	if( IIC->ICCR2.BIT.BBSY )				// Check I2C Bus Occupation
		return E_IO;					// I/O Error
	IIC->ICIER.BIT.TIE = 1;					// Enable TIE
	IIC->ICCR2.BIT.ST = 1;					// Generate Start Condition
	tk_wai_flg( iic->flgid, IIC_TXI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	IIC->ICDRT = devreq->start << 1;			// Set Slave Address + W
	for( i=0 ; i<devreq->size ; i++ )  {			// Data Length Loop
		tk_wai_flg( iic->flgid, IIC_TXI_INT | IIC_EEI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		if( flgptn & IIC_NACK_INT )			// NACK Receive ?
			break;
		IIC->ICDRT = ((UB*)devreq->buf)[i];		// Write Send Data
	}
	if( flgptn & IIC_NACK_INT )				// NACK Receive ?
		ercd = E_IO;					// I/O Error
	else  {
		IIC->ICIER.BIT.TEIE = 1;			// Enable TEIE
		tk_wai_flg( iic->flgid, IIC_TXI_INT | IIC_TEI_INT, TWF_ANDW | TWF_BITCLR, &flgptn, TMO_FEVR );
		IIC->ICIER.BIT.TEIE = 0;			// Disable TEIE
		ercd = E_OK;					// Normal End
	}
	IIC->ICIER.BIT.TIE = 0;					// Disable TIE
	IIC->ICCR2.BIT.SP = 1;					// Generate STOP Condition
	tk_wai_flg( iic->flgid, IIC_EEI_INT, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
	if( flgptn & IIC_NACK_INT )
		ercd = E_IO;
	return ercd;
}

LOCAL void IIC_Handler(UINT ch, UINT flgptn)
{
IIC_TBL *iic = &iic_tbl[ch];
	tk_set_flg( iic->flgid, flgptn );			// Set IIC Interrupt Event
}

#ifdef USE_IIC0
LOCAL void riic0_eei0_hdr(UINT dintno)
{
UINT flgptn;
	flgptn = RIIC0.ICSR2.BYTE;
	RIIC0.ICSR2.BYTE &= 0xE7;				// Clear NACKF,STOP fo ICSR2
	IIC_Handler( IIC0, flgptn & IIC_EEI_INT );		// Call IIC Handler
}

LOCAL void riic0_rxi0_hdr(UINT dintno)
{
	IIC_Handler( IIC0, IIC_RXI_INT );			// Call IIC Handler
}

LOCAL void riic0_txi0_hdr(UINT dintno)
{
	IIC_Handler( IIC0, IIC_TXI_INT );			// Call IIC Handler
}

LOCAL void riic0_tei0_hdr(UINT dintno)
{
	RIIC0.ICSR2.BIT.TEND = 0;				// Clear TEND fo ICSR2
	IIC_Handler( IIC0, IIC_TEI_INT );			// Call IIC Handler
}
#endif /* USE_IIC0 */

#ifdef USE_IIC1
LOCAL void riic1_eei1_hdr(UINT dintno)
{
UINT flgptn;
	flgptn = RIIC1.ICSR2.BYTE;
	RIIC1.ICSR2.BYTE &= 0xE7;				// Clear NACKF,STOP fo ICSR2
	IIC_Handler( IIC1, flgptn & IIC_EEI_INT );		// Call IIC Handler
}

LOCAL void riic1_rxi1_hdr(UINT dintno)
{
	IIC_Handler( IIC1, IIC_RXI_INT );			// Call IIC Handler
}

LOCAL void riic1_txi1_hdr(UINT dintno)
{
	IIC_Handler( IIC1, IIC_TXI_INT );			// Call IIC Handler
}

LOCAL void riic1_tei1_hdr(UINT dintno)
{
	RIIC1.ICSR2.BIT.TEND = 0;				// Clear TEND of ICSR2
	IIC_Handler( IIC1, IIC_TEI_INT );			// Call IIC Handler
}
#endif /* USE_IIC1 */

#ifdef USE_IIC2
LOCAL void riic2_eei2_hdr(UINT dintno)
{
UINT flgptn;
	flgptn = RIIC2.ICSR2.BYTE;
	RIIC2.ICSR2.BYTE &= 0xE7;				// Clear NACKF,STOP fo ICSR2
	IIC_Handler( IIC2, flgptn & IIC_EEI_INT );		// Call IIC Handler
}

LOCAL void riic2_rxi2_hdr(UINT dintno)
{
	IIC_Handler( IIC2, IIC_RXI_INT );			// Call IIC Handler
}

LOCAL void riic2_txi2_hdr(UINT dintno)
{
	IIC_Handler( IIC2, IIC_TXI_INT );			// Call IIC Handler
}

LOCAL void riic2_tei2_hdr(UINT dintno)
{
	RIIC2.ICSR2.BIT.TEND = 0;				// Clear TEND fo ICSR2
	IIC_Handler( IIC2, IIC_TEI_INT );			// Call IIC Handler
}
#endif /* USE_IIC2 */

LOCAL ER iicCreFlg(IIC_TBL *iic, T_CFLG *p_cflg)
{
	p_cflg->flgatr = TA_TPRI | TA_WMUL;			// Set EventFlag Attribute
#if USE_OBJECT_NAME
	p_cflg->flgatr |= TA_DSNAME;				// Set EventFlag Attribute
#ifdef CLANGSPEC
	strcpy( p_cflg->dsname, "iic*_f" );			// Set Debugger Suport Name
#else
	strcpy( (char*)p_cflg->dsname, "iic*_f" );		// Set Debugger Suport Name
#endif /* CLANGSPEC */
	p_cflg->dsname[3] = iic->drvname[3];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
	p_cflg->iflgptn = 0;					// Set Initial Bit Pattern
	return iic->flgid = tk_cre_flg( p_cflg );		// Create IIC EventFlag
}

LOCAL ER iicCreTsk(IIC_TBL *iic, UINT ch, T_CTSK *p_ctsk)
{
ID objid;
	p_ctsk->exinf = (void*)ch;				// Set Exinf(RIIC Channel Number)
	p_ctsk->tskatr  = TA_HLNG;				// Set Task Attribute
#if USE_OBJECT_NAME
	p_ctsk->tskatr |= TA_DSNAME;				// Set Task Attribute
#endif /* USE_OBJECT_NAME */
#if !USE_IMALLOC
	p_ctsk->tskatr |= TA_USERBUF;				// Set Task Attribute
	p_ctsk->bufptr = iic_task_stack[ch];			// Set Stack Top Address
#endif /* USE_OBJECT_NAME */
	p_ctsk->stksz = 300;					// Set Task StackSize
	p_ctsk->itskpri = IIC_CFG_TASK_PRIORITY;		// Set Task Priority
#ifdef CLANGSPEC
	p_ctsk->task = iic_tsk;					// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( p_ctsk->dsname, "iic*_t" );			// Set Task Debugger Suport Name
	p_ctsk->dsname[3] = iic->drvname[3];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
#else
	p_ctsk->task = (FP)iic_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)p_ctsk->dsname, "iic*_t" );		// Set Task Debugger Suport Name
	p_ctsk->dsname[3] = iic->drvname[3];			// Copy Channel Character Code
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( p_ctsk )) <= E_OK )		// Create IIC Control Task
		return objid;
	return tk_sta_tsk( objid, 0 );				// Start IIC Control Task
}

EXPORT ER iicDrvEntry(void)
{
ER ercd;
union { T_CTSK t_ctsk; T_CFLG t_cflg; T_DDEV t_ddev; T_DINT t_dint; } u;
#ifdef USE_IIC0
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( RIIC0 ) )  {				// RIIC0 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// RIIC0 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( RIIC0 ) = 0;					// Enable RIIC0
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	RIIC0.ICCR1.BIT.IICRST = 1;				// RIIC0 Reset
	RIIC0.ICCR1.BIT.ICE = 1;				// SCL0,SDA0 Pins in Active State
	RIIC0.ICSER.BYTE = 0x00;				// SARL0,SARU0,General ID is Ignore
	RIIC0.ICMR1.BIT.CKS = 2;				// Base Clock = PCLKB/4(60/4=15MHz)
	RIIC0.ICBRH.BIT.BRH = PCLKB/4.F/1000000*IIC0_HPSCL-1E-6F; // SCL0 High Pulse Width Minimum
	RIIC0.ICBRL.BIT.BRL = PCLKB/4.F/1000000*IIC0_LPSCL-1E-6F; // SCL0 Low  Pulse Width Minimum
	RIIC0.ICMR2.BYTE = 0x00;				// TMOL,TMOH is Ignore
//	RIIC0.ICMR2.BYTE = 0x10;				// TMOL,TMOH is Ignore, SDA Delay Counter is 1
	RIIC0.ICMR3.BYTE = 0x10;				// ACKBT Write Enable, Noise Filter is 1
//	RIIC0.ICMR3.BYTE = 0x11;				// ACKBT Write Enable, Noise Filter is 2
	RIIC0.ICFER.BYTE = 0x50;				// SCLE,NACKE Enable
	RIIC0.ICIER.BYTE = 0x18;				// NAKIE,SPIE Enable
	RIIC0.ICCR1.BIT.IICRST = 0;				// Release from Internal Reset State
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
	MPC.P12PFS.BYTE = 0x0F;					// P12 is SCL0 Pin
	MPC.P13PFS.BYTE = 0x0F;					// P13 is SDA0 Pin
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	PORT1.ODR0.BYTE |= 0x50;				// SCL0,SDA0 is Open Drain
	PORT1.PMR.BYTE |= 0x0C;					// P12,P13 is Peripheral Pin
	tk_ena_dsp( );						// Dispatch Enable
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = riic0_rxi0_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)riic0_rxi0_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( RIIC0, RXI0 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = riic0_txi0_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)riic0_txi0_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( RIIC0, TXI0 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( RIIC0, RXI0 ), IIC_CFG_INT_PRIORITY );	// Enable RIIC0 RXI0 Interrupt
	EnableInt( VECT( RIIC0, TXI0 ), IIC_CFG_INT_PRIORITY );	// Enable RIIC0 TXI0 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL1 ) )  {				// BL1 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL1Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL1Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL1 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL1 ), IIC_CFG_INT_PRIORITY );
	}							// Enable BL1 Interrupt
	GroupBL1Table[13] = riic0_tei0_hdr;			// Set Handler Address
	GroupBL1Table[14] = riic0_eei0_hdr;			// Set Handler Address
	EN( RIIC0, TEI0 ) = 1;					// Enable RIIC0 TEI0 Group Interrupt
	EN( RIIC0, EEI0 ) = 1;					// Enable RIIC0 EEI0 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL1 ) != IIC_CFG_INT_PRIORITY )	// BL1 Group IPR != IIC IPR ?
		return E_IO;
	iic_tbl[IIC0].drvname = "iica";				// Set Driver Name
	iic_tbl[IIC0].iic = (void*)&RIIC0;			// Set RIIC Channel Address
	iic_tbl[IIC0].next = IIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = iicCreFlg( &iic_tbl[IIC0], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create IIC EventFlag
	if( ( ercd = iicCreTsk( &iic_tbl[IIC0], IIC0, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create IIC Task
	if( ( ercd = iicDriverEntry( iic_tbl[IIC0].drvname, IIC0, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_IIC0 */
#ifdef USE_IIC1
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( RIIC1 ) )  {				// RIIC1 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// RIIC1 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( RIIC1 ) = 0;					// Enable RIIC1
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	RIIC1.ICCR1.BIT.IICRST = 1;				// RIIC2 Reset
	RIIC1.ICCR1.BIT.ICE = 1;				// SCL2,SDA2 Pins in Active State
	RIIC1.ICSER.BYTE = 0x00;				// SARL2,SARU2,General ID is Ignore
	RIIC1.ICMR1.BIT.CKS = 2;				// Base Clock = PCLKB/4(60/4=15MHz)
	RIIC1.ICBRH.BIT.BRH = PCLKB/4.F/1000000*IIC1_HPSCL-1E-6F; // SCL1 High Pulse Width Minimum
	RIIC1.ICBRL.BIT.BRL = PCLKB/4.F/1000000*IIC1_LPSCL-1E-6F; // SCL1 Low  Pulse Width Minimum
	RIIC1.ICMR2.BYTE = 0x00;				// TMOL,TMOH is Ignore
//	RIIC1.ICMR2.BYTE = 0x10;				// TMOL,TMOH is Ignore, SDA Delay Counter is 1
	RIIC1.ICMR3.BYTE = 0x10;				// ACKBT Write Enable, Noise Filter is 1
//	RIIC1.ICMR3.BYTE = 0x11;				// ACKBT Write Enable, Noise Filter is 2
	RIIC1.ICFER.BYTE = 0x50;				// SCLE,NACKE Enable
	RIIC1.ICIER.BYTE = 0x18;				// NAKIE,SPIE Enable
	RIIC1.ICCR1.BIT.IICRST = 0;				// Release from Internal Reset State
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
	MPC.P21PFS.BYTE = 0x0F;					// P21 is SCL1 Pin
	MPC.P20PFS.BYTE = 0x0F;					// P20 is SDA1 Pin
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	PORT2.ODR0.BYTE |= 0x05;				// SCL1,SDA1 is Open Drain
	PORT2.PMR.BYTE |= 0x03;					// P21,P20 is Peripheral Pin
	tk_ena_dsp( );						// Dispatch Enable
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = riic1_rxi1_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)riic1_rxi1_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( RIIC1, RXI1 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = riic1_txi1_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)riic1_txi1_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( RIIC1, TXI1 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( RIIC1, RXI1 ), IIC_CFG_INT_PRIORITY );	// Enable RIIC1 RXI1 Interrupt
	EnableInt( VECT( RIIC1, TXI1 ), IIC_CFG_INT_PRIORITY );	// Enable RIIC1 TXI1 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL1 ) )  {				// BL1 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL1Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL1Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL1 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL1 ), IIC_CFG_INT_PRIORITY );
	}							// Enable BL1 Interrupt
	GroupBL1Table[28] = riic1_tei1_hdr;			// Set Handler Address
	GroupBL1Table[29] = riic1_eei1_hdr;			// Set Handler Address
	EN( RIIC1, TEI1 ) = 1;					// Enable RIIC1 TEI1 Group Interrupt
	EN( RIIC1, EEI1 ) = 1;					// Enable RIIC1 EEI1 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL1 ) != IIC_CFG_INT_PRIORITY )	// BL1 Group IPR != IIC IPR ?
		return E_IO;
	iic_tbl[IIC1].drvname = "iicb";				// Set Driver Name
	iic_tbl[IIC1].iic = (void*)&RIIC1;			// Set RIIC Channel Address
	iic_tbl[IIC1].next = IIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = iicCreFlg( &iic_tbl[IIC1], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create IIC EventFlag
	if( ( ercd = iicCreTsk( &iic_tbl[IIC1], IIC1, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create IIC Task
	if( ( ercd = iicDriverEntry( iic_tbl[IIC1].drvname, IIC1, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_IIC1 */
#ifdef USE_IIC2
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( RIIC2 ) )  {				// RIIC2 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// RIIC2 is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( RIIC2 ) = 0;					// Enable RIIC2
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
	RIIC2.ICCR1.BIT.IICRST = 1;				// RIIC2 Reset
	RIIC2.ICCR1.BIT.ICE = 1;				// SCL2,SDA2 Pins in Active State
	RIIC2.ICSER.BYTE = 0x00;				// SARL2,SARU2,General ID is Ignore
	RIIC2.ICMR1.BIT.CKS = 2;				// Base Clock = PCLKB/4(60/4=15MHz)
	RIIC2.ICBRH.BIT.BRH = PCLKB/4.F/1000000*IIC2_HPSCL-1E-6F; // SCL2 High Pulse Width Minimum
	RIIC2.ICBRL.BIT.BRL = PCLKB/4.F/1000000*IIC2_LPSCL-1E-6F; // SCL2 Low  Pulse Width Minimum
	RIIC2.ICMR2.BYTE = 0x00;				// TMOL,TMOH is Ignore
//	RIIC2.ICMR2.BYTE = 0x10;				// TMOL,TMOH is Ignore, SDA Delay Counter is 1
	RIIC2.ICMR3.BYTE = 0x10;				// ACKBT Write Enable, Noise Filter is 1
//	RIIC2.ICMR3.BYTE = 0x11;				// ACKBT Write Enable, Noise Filter is 2
	RIIC2.ICFER.BYTE = 0x50;				// SCLE,NACKE Enable
	RIIC2.ICIER.BYTE = 0x18;				// NAKIE,SPIE Enable
	RIIC2.ICCR1.BIT.IICRST = 0;				// Release from Internal Reset State
	tk_dis_dsp( );						// Dispatch Disable
	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable
	MPC.P16PFS.BYTE = 0x0F;					// P16 is SCL2 Pin
	MPC.P17PFS.BYTE = 0x0F;					// P17 is SDA2 Pin
	MPC.PWPR.BYTE = 0x80;					// Write Disable
	PORT1.ODR1.BYTE |= 0x50;				// SCL2,SDA2 is Open Drain
	PORT1.PMR.BYTE |= 0xC0;					// P16,P17 is Peripheral Pin
	tk_ena_dsp( );						// Dispatch Enable
	u.t_dint.intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	u.t_dint.inthdr = riic2_rxi2_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)riic2_rxi2_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( RIIC2, RXI2 ), &u.t_dint );		// Define Interrupt Handler
#ifdef CLANGSPEC
	u.t_dint.inthdr = riic2_txi2_hdr;			// Set Handler Address
#else
	u.t_dint.inthdr = (FP)riic2_txi2_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( RIIC2, TXI2 ), &u.t_dint );		// Define Interrupt Handler
	EnableInt( VECT( RIIC2, RXI2 ), IIC_CFG_INT_PRIORITY );	// Enable RIIC2 RXI2 Interrupt
	EnableInt( VECT( RIIC2, TXI2 ), IIC_CFG_INT_PRIORITY );	// Enable RIIC2 TXI2 Interrupt
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPBL1 ) )  {				// BL1 Group IPR is Zero ?
#ifdef CLANGSPEC
		u.t_dint.inthdr = GroupBL1Handler;		// Set Handler Address
#else
		u.t_dint.inthdr = (FP)GroupBL1Handler;		// Set Handler Address
#endif
		tk_def_int( VECT( ICU, GROUPBL1 ), &u.t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPBL1 ), IIC_CFG_INT_PRIORITY );
	}							// Enable BL1 Interrupt
	GroupBL1Table[15] = riic2_tei2_hdr;			// Set Handler Address
	GroupBL1Table[16] = riic2_eei2_hdr;			// Set Handler Address
	EN( RIIC2, TEI2 ) = 1;					// Enable RIIC2 TEI2 Group Interrupt
	EN( RIIC2, EEI2 ) = 1;					// Enable RIIC2 EEI2 Group Interrupt
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPBL1 ) != IIC_CFG_INT_PRIORITY )	// BL1 Group IPR != IIC IPR ?
		return E_IO;
	iic_tbl[IIC2].drvname = "iicc";				// Set Driver Name
	iic_tbl[IIC2].iic = (void*)&RIIC2;			// Set RIIC Channel Address
	iic_tbl[IIC2].next = IIC_MAXIMUM;			// Set Next Command Pointer
	if( ( ercd = iicCreFlg( &iic_tbl[IIC2], &u.t_cflg ) ) < E_OK )
		goto ERROR;					// Create IIC EventFlag
	if( ( ercd = iicCreTsk( &iic_tbl[IIC2], IIC2, &u.t_ctsk ) ) < E_OK )
		goto ERROR;					// Create IIC Task
	if( ( ercd = iicDriverEntry( iic_tbl[IIC2].drvname, IIC2, &u.t_ddev ) ) < E_OK )
		goto ERROR;					// Define Device Driver
#endif /* USE_IIC2 */
	return E_OK;
ERROR:
	return ercd;
}