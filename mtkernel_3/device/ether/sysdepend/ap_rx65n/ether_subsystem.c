/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2022/11/24.
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
	MPC.P77PFS.BYTE = 0x12;				// P77 is RMII0_RX_ER
	MPC.P76PFS.BYTE = 0x12;				// P76 is REF50CK0
	MPC.P75PFS.BYTE = 0x12;				// P75 is RMII0_RXD0
	MPC.P74PFS.BYTE = 0x12;				// P74 is RMII0_RXD1
	MPC.P72PFS.BYTE = 0x11;				// P72 is ET0_MDC
	MPC.P71PFS.BYTE = 0x11;				// P71 is ET0_MDIO
	MPC.P83PFS.BYTE = 0x12;				// P83 is RMII0_CRS_DV
	MPC.P82PFS.BYTE = 0x12;				// P82 is RMII0_TXD1
	MPC.P81PFS.BYTE = 0x12;				// P81 is RMII0_TXD0
	MPC.P80PFS.BYTE = 0x12;				// P80 is RMII0_TXD_EN
	MPC.PWPR.BYTE = 0x80;				// Write Protect Enable
	PORT7.PMR.BYTE |= 0xF6;				// P77-P71(^P73) is Peripheral Pin
	PORT8.PMR.BYTE |= 0x0F;				// P83-P80 is Peripheral Pin
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
	if( IS( EDMAC0, EINT0 ) )
		eint_callback( NULL );			// Call EDMAC0 EINT0 Interrupt Handler
	else if( IS( GLCDC, VPOS ) )
		;					// Call GLCDC VPOS  Interrupt Handler
	else if( IS( GLCDC, GR1UF ) )
		;					// Call GLCDC GR1UF Interrupt Handler
	else if( IS( GLCDC, GR2UF ) )
		;					// Call GLCDC GR2UF Interrupt Handler
	else if( IS( DRW2D, DRWIRQ ) )
		;					// Call DRW2D DRW_IRQ Interrupt Handler
}

bsp_int_err_t R_BSP_InterruptWrite(bsp_int_src_t vector,  bsp_int_cb_t callback)
{
T_DINT t_dint;
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