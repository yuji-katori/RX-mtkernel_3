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
 *	CPU dependent definition  (RXv1 core depended)
 */

#ifndef __TK_CPUDEF_CORE_H__
#define __TK_CPUDEF_CORE_H__

#include <config.h>

/*
 * Using FPU (depend on CPU)
 *   0: not using FPU
 *   TA_COPn(n = 0-3): using FPU
 */
#define TA_FPU		0

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
	UD	acc;		/* Accumulator */
#endif
#if USE_FPU == 1
	UW	fpsw;		/* Floating Point Status register */
#endif
	void	*sp;		/* System stack pointer SP(USP) */
} T_CREGS;

#endif /* __TK_CPUDEF_CORE_H__ */
