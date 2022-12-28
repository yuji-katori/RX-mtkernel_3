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
//	
	MPC.PWPR.BYTE = 0x80;				// Write Protect Enable
//	
#elif	ETHER_CFG_MODE_SEL == 1				// RMII
	MPC.PWPR.BIT.B0WI = 0;				// Write Protect Disable
	MPC.PWPR.BIT.PFSWE = 1;
	MPC.P34PFS.BYTE = 0x11;				// P34 is ET0_LINKSTA
	MPC.P72PFS.BYTE = 0x11;				// P72 is ET0_MDC
	MPC.P71PFS.BYTE = 0x11;				// P71 is ET0_MDIO
	MPC.PL6PFS.BYTE = 0x12;				// PL6 is RMII0_TXD_EN
	MPC.PL5PFS.BYTE = 0x12;				// PL5 is RMII0_TXD1
	MPC.PL4PFS.BYTE = 0x12;				// PL4 is RMII0_TXD0
	MPC.PL3PFS.BYTE = 0x12;				// PL3 is REF50CK0
	MPC.PL2PFS.BYTE = 0x12;				// PL2 is RMII0_RX_ER
	MPC.PL1PFS.BYTE = 0x12;				// PL1 is RMII0_RXD1
	MPC.PL0PFS.BYTE = 0x12;				// PL0 is RMII0_RXD0
	MPC.PM7PFS.BYTE = 0x12;				// PM7 is RMII0_CRS_DV
	MPC.PWPR.BYTE = 0x80;				// Write Protect Enable
	PORT3.PMR.BYTE |= 0x10;				// P34 is Peripheral Pin
	PORT7.PMR.BYTE |= 0x06;				// P72-P71 is Peripheral Pin
	PORTL.PMR.BYTE |= 0x7F;				// PL6-PL0 is Peripheral Pin
	PORTM.PMR.BYTE |= 0x80;				// PM7 is Peripheral Pin
#endif
	tk_ena_dsp( );					// Dispatch Enable

	u.t_ctsk.tskatr = TA_HLNG | TA_DSNAME;		// Set Task Attribute
	u.t_ctsk.stksz = 512;				// Set Task StackSize
	u.t_ctsk.itskpri = ETHER_CFG_TASK_PRIORTY;	// Set Task Priority
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

void ether_disable_icu(UW channel)
{
	ICU.GENAL1.BIT.EN4 = 0;				// Ether Interrupt Disable
}

void ether_enable_icu(UW channel)
{
	ICU.GENAL1.BIT.EN4 = 1;				// Ether Interrupt Enable
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
T_DINT t_dint;
	if( vector == BSP_INT_SRC_AL1_EDMAC1_EINT1 )	// If Ether1 then nothing.
		return BSP_INT_SUCCESS;
	EnableInt( VECT( ICU, GROUPAL1 ), ETHER_CFG_INT_PRIORTY );
	t_dint.intatr = TA_HLNG;			// Set Interrupt Handler Attribute
#ifdef CLANGSPEC
	t_dint.inthdr = (INTFP)callback;		// Set Handler Start Address
#else
	t_dint.inthdr = (FP)callback;			// Set Handler Start Address
#endif
	if( tk_def_int( VECT( ICU, GROUPAL1 ), &t_dint ) != E_OK )	// Define Interrupt Handler
		goto ERROR;
	return BSP_INT_SUCCESS;
ERROR:
	while( 1 )  ;		
}

/*
static bsp_int_cb_t eint_callback;
void GroupAL1Handler(UINT intno)
{
	if( IS( EPTPC, MINT ) )
		;					// Call EPTPC MINT Interrupt Handler
	else if( IS( PTPEDMAC, PINT ) )
		;					// Call PTPEDMAC PINT Interrupt Handler
	else if( IS( EDMAC0, EINT0 ) )
		eint_callback( NULL );			// Call EDMAC0 EINT0 Interrupt Handler
	else if( IS( EDMAC1, EINT1 ) )
		;					// Call EDMAC1 EINT1 Interrupt Handler
	else if( IS( GLCDC, VPOS ) )
		;					// Call GLCDC VPOS  Interrupt Handler
	else if( IS( GLCDC, GR1UF ) )
		;					// Call GLCDC GR1UF Interrupt Handler
	else if( IS( GLCDC, GR2UF ) )
		;					// Call GLCDC GR2UF Interrupt Handler
}

bsp_int_err_t R_BSP_InterruptWrite(bsp_int_src_t vector,  bsp_int_cb_t callback)
{
T_DINT t_dint;
	if( vector == BSP_INT_SRC_AL1_EDMAC1_EINT1 )	// If Ether1 then nothing.
		return BSP_INT_SUCCESS;
	eint_callback = callback;
	EnableInt( VECT( ICU, GROUPAL1 ), ETHER_CFG_INT_PRIORTY );
	t_dint.intatr = TA_HLNG;			// Set Interrupt Handler Attribute
#ifdef CLANGSPEC
	t_dint.inthdr = GroupAL1Handler;		// Set Handler Start Address
#else
	t_dint.inthdr = (FP)GroupAL1Handler;		// Set Handler Start Address
#endif
	if( tk_def_int( VECT( ICU, GROUPAL1 ), &t_dint ) != E_OK )	// Define Interrupt Handler
		goto ERROR;
	return BSP_INT_SUCCESS;
ERROR:
	while( 1 )  ;		
}
*/