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

/*
 *	cpu_task.h (RXv2)
 *	CPU-Dependent Task Start Processing
 */

#ifndef _SYSDEPEND_CPU_CORE_CPUTASK_
#define _SYSDEPEND_CPU_CORE_CPUTASK_

/*
 * System stack configuration at task startup
 */
typedef struct {
#if USE_DSP == 1
	UW	acc0[3];	/* ACC0   */
	UW	acc1[3];	/* ACC1   */
#endif
#if USE_FPU == 1
	UW	fpsw;		/* FPSW   */
#endif
	UW	r[15];		/* R1-R15 */
	UW	pc;		/* PC     */
	UW	psw;		/* PSW    */
} SStackFrame;

/*
 * Size of system stack area destroyed by 'make_dormant()'
 * In other words, the size of area required to write by 'setup_context().'
 */
//#define DORMANT_STACK_SIZE	( sizeof(VW) * 1 )	/* To 'spc,spsw' position */

/*
 * Initial value for task startup
 */
#define INIT_PSW	( 0x00030000 )


/*
 * Create stack frame for task startup
 *	Call from 'make_dormant()'
 */
Inline void knl_setup_context( TCB *tcb )
{
	SStackFrame	*ssp;

	ssp = tcb->isstack;
	ssp--;

	/* CPU context initialization */
		  /* Initial FPSW */
#if USE_FPU == 1
	ssp->fpsw = 0x00000100;
#endif
		  /* Task startup address */
	ssp->pc = (UW)tcb->task;
		  /* Initial PSW */
	ssp->psw = INIT_PSW;
	tcb->tskctxb.ssp = ssp;		/* System stack */
}

/*
 * Set task startup code
 *	Called by 'tk_sta_tsk()' processing.
 */
Inline void knl_setup_stacd( TCB *tcb, INT stacd )
{
	SStackFrame	*ssp = tcb->tskctxb.ssp;

	ssp->r[0] = stacd;			/* R1 */
	ssp->r[1] = (UW)tcb->exinf;		/* R2 */
}

/*
 * Delete task contexts
 */
Inline void knl_cleanup_context( TCB *tcb )
{
}

#endif /* _SYSDEPEND_CPU_CORE_CPUTASK_ */
