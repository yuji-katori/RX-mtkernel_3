/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2019 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 */
#include <sys/machine.h>
#ifdef CPU_CORE_RXV3
/*
 *	cpu_cntl.c (RXv3)
 *	CPU-Dependent Control
 */
#include "kernel.h"
#include "../../../sysdepend.h"

#include "cpu_task.h"

/* ------------------------------------------------------------------------ */
/*
 * Set task register contents (Used in tk_set_reg())
 */
EXPORT void knl_set_reg( TCB *tcb, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs )
{
	SStackFrame	*ssp;
	INT	i;

	ssp = cregs != NULL ? cregs->sp : tcb->tskctxb.ssp;

#if TK_SUPPORT_FPU
	if( tcb->tskatr & TA_FPU )  {
		SStackFrame_DFPU	*dfpu;
		dfpu = (SStackFrame_DFPU *)ssp;
		dfpu ++ ;
		ssp = (SStackFrame *)dfpu;
	}
#endif	/* TK_SUPPORT_FPU */

	if ( cregs != NULL )
		ssp = cregs->sp;

	if ( regs != NULL )
		for ( i = 0; i < 15 ; i++ )
			ssp->r[i] = regs->r[i];

	if ( eit != NULL )
		ssp->pc = eit->pc,
		ssp->psw = eit->psw;

	if ( cregs != NULL )  {
#if USE_FPU == 1
		ssp->fpsw = cregs->fpsw;
#endif
#if USE_DSP == 1
		for( i = 0 ; i < 3 ; i++ )
			ssp->acc0[i] = cregs->acc0[i],
			ssp->acc1[i] = cregs->acc1[i];
#endif
		tcb->tskctxb.ssp = cregs->sp;
	}
}


/* ------------------------------------------------------------------------ */
/*
 * Get task register contents (Used in tk_get_reg())
 */
EXPORT void knl_get_reg( TCB *tcb, T_REGS *regs, T_EIT *eit, T_CREGS *cregs )
{
	SStackFrame	*ssp;
	INT		i;

	ssp = tcb->tskctxb.ssp;
#if TK_SUPPORT_FPU
	if( tcb->tskatr & TA_FPU )  {
		SStackFrame_DFPU	*dfpu;
		dfpu = (SStackFrame_DFPU *)ssp;
		dfpu ++ ;
		ssp = (SStackFrame *)dfpu;
	}
#endif /* TK_SUPPORT_FPU */

	if ( regs != NULL )
		for ( i = 0 ; i < 15 ; i++ )
			regs->r[i] = ssp->r[i];

	if ( eit != NULL )
		eit->pc  = ssp->pc,
		eit->psw = ssp->psw;

	if ( cregs != NULL )  {
#if USE_FPU == 1
		cregs->fpsw = ssp->fpsw;
#endif
#if USE_DSP == 1
		for ( i = 0 ; i < 3 ; i++ )
			cregs->acc0[i] = ssp->acc0[i],
			cregs->acc1[i] = ssp->acc1[i];
#endif
		cregs->sp = tcb->tskctxb.ssp;
	}
}

#if TK_SUPPORT_FPU
#ifdef USE_FUNC_TK_SET_CPR
/* ------------------------------------------------------------------------ */
/*
 * Set task register contents (Used in tk_set_reg())
 */
EXPORT ER knl_set_cpr( TCB *tcb, INT copno, CONST T_COPREGS *copregs )
{
	SStackFrame_DFPU	*dfpu;
	INT		i;

	dfpu = tcb->tskctxb.ssp;

	if( copregs != NULL )  {
		for ( i = 0 ; i < 16 ; i++ )
			dfpu->dr[i] = copregs->dr[i];
		dfpu->dpsw  = copregs->dpsw;
		dfpu->dcmr  = copregs->dcmr;
		dfpu->decnt = copregs->decnt;
//		dfpu->depc  = copregs->depc;
	}

	return E_OK;
}

#endif /* USE_FUNC_TK_SET_CPR */

#ifdef USE_FUNC_TK_GET_CPR
/* ------------------------------------------------------------------------ */
/*
 * Get task FPU register contents (Used in tk_get_cpr())
 */
EXPORT ER knl_get_cpr( TCB *tcb, INT copno, T_COPREGS *copregs)
{
	SStackFrame_DFPU	*dfpu;
	INT		i;

	dfpu = tcb->tskctxb.ssp;

	if( copregs != NULL )  {
		for ( i = 0 ; i < 16 ; i++ )
			copregs->dr[i] = dfpu->dr[i];
		copregs->dpsw  = dfpu->dpsw;
		copregs->dcmr  = dfpu->dcmr;
		copregs->decnt = dfpu->decnt;
		copregs->depc  = dfpu->depc;
	}

	return E_OK;
}
#endif /* USE_FUNC_TK_GET_CPR */
#endif /* TK_SUPPORT_FPU */

#endif /* CPU_CORE_RXV3 */
