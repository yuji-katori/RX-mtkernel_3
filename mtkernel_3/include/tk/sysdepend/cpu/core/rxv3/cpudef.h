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
 *	cpudef.h
 *
 *	CPU dependent definition  (RXv3 core depended)
 */

#ifndef __TK_CPUDEF_CORE_H__
#define __TK_CPUDEF_CORE_H__

#include <config.h>

/*
 * Using FPU (depend on CPU)
 *   0: not using FPU
 *   TA_COPn(n = 0-3): using FPU
 */
#define TA_FPU		TA_COP0

/*
 * General purpose register		tk_get_reg tk_set_reg
 */
typedef struct t_regs {
	UW	r[15];		/* General purpose register R1-R15 */
} T_REGS;

/*
 * Exception-related register		tk_get_reg tk_set_reg
 */
typedef struct t_eit {
	UW	pc;		/* Program counter */
	UW	psw;		/* Status register */
} T_EIT;

/*
 * Control register			tk_get_reg tk_set_reg
 */
typedef struct t_cregs {
#if USE_DSP == 1
	UW	acc0[3];	/* Accumulator 0 */
	UW	acc1[3];	/* Accumulator 1 */
#endif
#if USE_FPU == 1
	UW	fpsw;		/* Floating Point Status register */
#endif
	void	*sp;		/* System stack pointer SP(USP) */
} T_CREGS;

#if TK_SUPPORT_FPU
/*
 * Coprocessor registers
 */
typedef struct t_copregs {
	UD	dr[16];		/* FPU General purpose register d0-d31 */
	UW	dpsw;		/* Double-Precision Floating-Point Status Word */
	UW	dcmr;		/* Double-Precision Floating-Point Comparison Result Register */
	UW	decnt;		/* Double-Precision Floating-Point Exception Handling Control Register */
	UW	depc;		/* Double-Precision Floating-Point Exception Program Counter */
} T_COPREGS;
#endif  /* TK_SUPPORT_FPU  */

#endif /* __TK_CPUDEF_CORE_H__ */
