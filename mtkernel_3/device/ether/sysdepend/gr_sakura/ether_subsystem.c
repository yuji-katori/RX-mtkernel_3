/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	ether_subsystem.c
 */

#include <string.h>
#include <tk/tkernel.h>
#include "dev_ether.h"
#include "platform.h"
#include "ether_driver.h"
#include "r_ether_rx_if.h"
#include "config/config_tcpudp.h"

ID ether_objid[2];
#if !USE_IMALLOC
LOCAL INT ether_task_stack[512/sizeof(INT)];
#endif /* USE_IMALLOC */

void Initialize_Ether(void)
{
ID objid;
union { T_CTSK t_ctsk; T_CCYC t_ccyc; } u;
	tk_dis_dsp( );					// Dispatch Disable
#if	ETHER_CFG_MODE_SEL == 0				// MII
	MPC.PWPR.BIT.B0WI = 0;				// Write Protect Disable
	MPC.PWPR.BIT.PFSWE = 1;
//	
	MPC.PWPR.BYTE = 0x80;				// Write Protect Enable
//	
#elif	ETHER_CFG_MODE_SEL == 1				// RMII
	MPC.PWPR.BIT.B0WI = 0;				// Write Protect Disable
	MPC.PWPR.BIT.PFSWE = 1;
	MPC.PB7PFS.BYTE = 0x12;				// PB7 is RMII_CRS_DV
	MPC.PB6PFS.BYTE = 0x12;				// PB6 is RMII_TXD1
	MPC.PB5PFS.BYTE = 0x12;				// PB5 is RMII_TXD0
	MPC.PB4PFS.BYTE = 0x12;				// PB4 is RMII_TXD_EN
	MPC.PB3PFS.BYTE = 0x12;				// PB3 is RMII_RX_ER
	MPC.PB2PFS.BYTE = 0x12;				// PB2 is REF50CK
	MPC.PB1PFS.BYTE = 0x12;				// PB1 is RMII_RXD0
	MPC.PB0PFS.BYTE = 0x12;				// PB0 is RMII_RXD1
	MPC.PA4PFS.BYTE = 0x11;				// PA4 is ET0_MDC
	MPC.PA3PFS.BYTE = 0x11;				// PA3 is ET0_MDIO
	MPC.PWPR.BYTE = 0x80;				// Write Protect Enable
	PORTB.PMR.BYTE = 0xFF;				// PB7-PB0 is Peripheral Pin
	PORTA.PMR.BYTE |= 0x18;				// PA4-PA3 is Peripheral Pin
#endif
	tk_ena_dsp( );					// Dispatch Enable

	u.t_ctsk.tskatr  = TA_HLNG;			// Set Task Attribute
#if USE_OBJECT_NAME
	u.t_ctsk.tskatr |= TA_DSNAME;			// Set Task Attribute
#endif /* USE_OBJECT_NAME */
#if !USE_IMALLOC
	u.t_ctsk.tskatr |= TA_USERBUF;			// Set Task Attribute
	u.t_ctsk.bufptr = ether_task_stack;		// Set Stack Top Address
#endif /* USE_OBJECT_NAME */
	u.t_ctsk.stksz = 512;				// Set Task StackSize
	u.t_ctsk.itskpri = ETHER_CFG_TASK_PRIORTY;	// Set Task Priority
#ifdef CLANGSPEC
	u.t_ctsk.task =  ether_tsk;			// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( u.t_ctsk.dsname, "ether_t" );		// Set Task debugger Suport Name
#endif /* USE_OBJECT_NAME */
#else
	u.t_ctsk.task =  (FP)ether_tsk;			// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)u.t_ctsk.dsname, "ether_t" );	// Set Task debugger Suport Name
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( &u.t_ctsk )) <= E_OK )	// Create Ether Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) < E_OK )		// Start Ether Task
		goto ERROR;
	ether_objid[0] = objid;				// Set Ether Task ID

	u.t_ccyc.cycatr  = TA_HLNG;			// Set Cyclic Handler Attribute
#if USE_OBJECT_NAME
	u.t_ccyc.cycatr |= TA_DSNAME;			// Set Cyclic Handler Attribute
#endif /* USE_OBJECT_NAME */
#ifdef CLANGSPEC
	u.t_ccyc.cychdr = ether_cychdr;			// Set Cyclic Handler Address
#if USE_OBJECT_NAME
	strcpy( u.t_ccyc.dsname, "ether_c" );		// Set Task debugger Suport Name
#endif /* USE_OBJECT_NAME */
#else
	u.t_ccyc.cychdr = (FP)ether_cychdr;		// Set Cyclic Handler Address
#if USE_OBJECT_NAME
	strcpy( (char*)u.t_ccyc.dsname, "ether_c" );	// Set Task debugger Suport Name
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	u.t_ccyc.cyctim = 10;				// Set Cyclic Time
	u.t_ccyc.cycphs = 10;				// Set Cyclic Phase
	if( (objid = tk_cre_cyc( &u.t_ccyc )) <= E_OK )	// Create Ether Cyclic Handler
		goto ERROR;
	ether_objid[1] = objid;				// Set Ether Cyclic Handler ID
	return ;
ERROR:
	while( 1 )  ;		
}

void ether_disable_icu(UW channel)
{
	IEN( ETHER, EINT ) = 0;				// Ether Interrupt Disable
}

void ether_enable_icu(UW channel)
{
	IEN( ETHER, EINT ) = 1;				// Ether Interrupt Enable
}

void ether_set_phy_mode(UB connect)
{
#if	ETHER_CFG_MODE_SEL == 0				// MII
	MPC.PFENET.BIT.PHYMODE = 1;
#elif	ETHER_CFG_MODE_SEL == 1				// RMII
	MPC.PFENET.BIT.PHYMODE = 0;
#endif
}

bsp_int_err_t R_BSP_InterruptWrite(bsp_int_src_t vector,  bsp_int_cb_t callback)
{
T_DINT t_dint;
	IPR( ETHER, EINT ) = ETHER_CFG_INT_PRIORTY;	// Set Interrupt Level
	t_dint.intatr = TA_HLNG;			// Set Interrupt Handler Attribute
#ifdef CLANGSPEC
	t_dint.inthdr = (INTFP)callback;		// Set Handler Start Address
#else
	t_dint.inthdr = (FP)callback;			// Set Handler Start Address
#endif /* CLANGSPEC */
	if( tk_def_int( VECT( ETHER, EINT ), &t_dint ) != E_OK )	// Define Interrupt Handler
		goto ERROR;
	return BSP_INT_SUCCESS;
ERROR:
	while( 1 )  ;		
}