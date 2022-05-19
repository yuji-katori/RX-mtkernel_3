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
#ifdef CPU_CORE_RXV2
/*
 *	cpu_cntl.c (RXv2)
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

#endif /* CPU_CORE_RXV2 */
