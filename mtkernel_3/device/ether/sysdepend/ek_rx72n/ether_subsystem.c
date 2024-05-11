/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2024/04/27.
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

void Initialize_Ether(void)
{
ID objid;
union { T_CTSK t_ctsk; T_CCYC t_ccyc; } u;
	if( ether_objid[0] )
		return;
	tk_dis_dsp( );					// Dispatch Disable
#if	ETHER_CFG_MODE_SEL == 0				// MII
	MPC.PWPR.BIT.B0WI = 0;				// Write Protect Disable
	MPC.PWPR.BIT.PFSWE = 1;
	MPC.P56PFS.BYTE = 0x2A;				// P56 is CLKOUT25M
	MPC.P71PFS.BYTE = 0x28;				// P71 is PMGI0_MDIO
	MPC.P72PFS.BYTE = 0x28;				// P72 is PMGI0_MDC
	MPC.P74PFS.BYTE = 0x11;				// P74 is ET0_ERXD1
	MPC.P75PFS.BYTE = 0x11;				// P75 is ET0_ERXD0
	MPC.P76PFS.BYTE = 0x11;				// P76 is ET0_RX_CLK
	MPC.P77PFS.BYTE = 0x11;				// P77 is ET0_RX_ER
	MPC.P80PFS.BYTE = 0x11;				// P80 is ET0_TX_EN
	MPC.P81PFS.BYTE = 0x11;				// P81 is ET0_ETXD0
	MPC.P82PFS.BYTE = 0x11;				// P82 is ET0_ETXD1
	MPC.P83PFS.BYTE = 0x11;				// P83 is ET0_CRS
	MPC.PC0PFS.BYTE = 0x11;				// PC0 is ET0_ERXD3
	MPC.PC1PFS.BYTE = 0x11;				// PC1 is ET0_ERXD2
	MPC.PC2PFS.BYTE = 0x11;				// PC2 is ET0_RX_DV
	MPC.PC3PFS.BYTE = 0x11;				// PC3 is ET0_EX_ER
	MPC.PC4PFS.BYTE = 0x11;				// PC4 is ET0_TX_CLK
	MPC.PC5PFS.BYTE = 0x11;				// PC5 is ET0_ETXD2
	MPC.PC6PFS.BYTE = 0x11;				// PC6 is ET0_ETXD3
	MPC.PC7PFS.BYTE = 0x11;				// PC7 is ET0_COL
	MPC.PWPR.BYTE = 0x80;				// Write Protect Enable
	PORT5.PMR.BYTE |= 0x40;				// P56 is Peripheral Pin
	PORT7.PMR.BYTE |= 0xF6;				// P71,P72,P74-P77 is Peripheral Pin
	PORT8.PMR.BYTE |= 0x0F;				// P80-P83 is Peripheral Pin
	PORTC.PMR.BYTE |= 0xFF;				// PC0-PC7 is Peripheral Pin
#elif	ETHER_CFG_MODE_SEL == 1				// RMII
	MPC.PWPR.BIT.B0WI = 0;				// Write Protect Disable
	MPC.PWPR.BIT.PFSWE = 1;
//	
	MPC.PWPR.BYTE = 0x80;				// Write Protect Enable
//	
#endif
	tk_ena_dsp( );					// Dispatch Enable

	u.t_ctsk.tskatr = TA_HLNG | TA_DSNAME;		// Set Task Attribute
	u.t_ctsk.stksz = 512;				// Set Task StackSize
	u.t_ctsk.itskpri = ETHER_CFG_TASK_PRIORITY;	// Set Task Priority
#ifdef CLANGSPEC
	u.t_ctsk.task =  ether_tsk;			// Set Task Start Address
	strcpy( u.t_ctsk.dsname, "ether" );		// Set Task debugger Suport Name
#else
	u.t_ctsk.task =  (FP)ether_tsk;			// Set Task Start Address
	strcpy( (char*)u.t_ctsk.dsname, "ether" );	// Set Task debugger Suport Name
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( &u.t_ctsk )) <= E_OK )	// Create Ether Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) < E_OK )		// Start Ether Task
		goto ERROR;
	ether_objid[0] = objid;				// Set Ether Task ID

	u.t_ccyc.cycatr = TA_HLNG;			// Set Cyclic Handler Attribute
#ifdef CLANGSPEC
	u.t_ccyc.cychdr = ether_cychdr;			// Set Cyclic Handler Address
	strcpy( u.t_ccyc.dsname, "ether" );		// Set Task debugger Suport Name
#else
	u.t_ccyc.cychdr = (FP)ether_cychdr;		// Set Cyclic Handler Address
	strcpy( (char*)u.t_ccyc.dsname, "ether" );	// Set Task debugger Suport Name
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

void InterruptRequestDisable(UINT intno)
{
	IEN( PERIA, INTA208 ) = 0;			// PMGI Interrupt Disable
	ICU.SLIAR208.BYTE = 0;				// Clear Interrupt Factor
}

void InterruptRequestEnable(UINT intno)
{
	ICU.SLIAR208.BYTE = 98;				// Set Interrupt Factor
	IEN( PERIA, INTA208 ) = 1;			// PMGI Interrupt Enable
}

bool R_BSP_SoftwareLock(BSP_CFG_USER_LOCKING_TYPE *plock)
{
int32_t is_locked = true;
	__xchg( &is_locked, (int32_t*)&plock->lock );
	if( false == is_locked )
		return true;
	else
		return false;
}

bool R_BSP_SoftwareUnlock(BSP_CFG_USER_LOCKING_TYPE * plock)
{
	plock->lock = false;
	return true;
}

void ether_disable_icu(UW channel)
{
	tk_dis_dsp( );					// Dispatch Disable
	EN( EDMAC0, EINT0 ) = 0;			// Ether Interrupt Disable
	tk_ena_dsp( );					// Dispatch Enable
}

void ether_enable_icu(UW channel)
{
	tk_dis_dsp( );					// Dispatch Disable
	EN( EDMAC0, EINT0 ) = 1;			// Ether Interrupt Enable
	tk_ena_dsp( );					// Dispatch Enable
}

void ether_set_phy_mode(UB connect)
{
#if	ETHER_CFG_MODE_SEL == 0				// MII
	MPC.PFENET.BIT.PHYMODE0 = 1;
#elif	ETHER_CFG_MODE_SEL == 1				// RMII
	MPC.PFENET.BIT.PHYMODE0 = 0;
#endif
}

bsp_int_err_t R_BSP_InterruptWrite(bsp_int_src_t vector,  bsp_int_cb_t callback)
{
IMPORT void (*GroupAL1Table[])(UINT dintno);
IMPORT void GroupAL1Handler(UINT dintno);
T_DINT t_dint;
	t_dint.intatr = TA_HLNG;				// Set Interrupt Handler Attribute
#ifdef CLANGSPEC
	t_dint.inthdr = (INTFP)ether_pmgi0i_isr;		// Set Handler Start Address
#else
	t_dint.inthdr = (FP)ether_pmgi0i_isr;			// Set Handler Start Address
#endif
	tk_def_int( VECT( PERIA, INTA208 ), &t_dint );		// Define Interrupt Handler
	tk_dis_dsp( );						// Dispatch Disable
	if( ! IPR( ICU, GROUPAL1 ) )  {				// BL1 Group IPR is Zero ?
#ifdef CLANGSPEC
		t_dint.inthdr = GroupAL1Handler;		// Set Handler Start Address
#else
		t_dint.inthdr = (FP)GroupAL1Handler;		// Set Handler Start Address
#endif
		tk_def_int( VECT( ICU, GROUPAL1 ), &t_dint );	// Define Interrupt Handler
		EnableInt( VECT( ICU, GROUPAL1 ), ETHER_CFG_INT_PRIORITY );	// Enable BL1 Group Interrupt
	}
	GroupAL1Table[4] = (INTFP)callback;			// Set EDMAC0, EINT0 Handler Address
	tk_ena_dsp( );						// Dispatch Enable
	if( IPR( ICU, GROUPAL1 ) != ETHER_CFG_INT_PRIORITY )	// AL1 Group IPR != EDMAC IPR ?
		return BSP_INT_ERR_INVALID_IPL;
	return BSP_INT_SUCCESS;
}