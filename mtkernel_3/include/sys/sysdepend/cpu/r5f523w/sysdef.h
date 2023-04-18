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
 *	sysdef.h
 *
 *	System dependencies definition (R5F523W depended)
 *	Included also from assembler program.
 */

#ifndef __TK_SYSDEF_DEPEND_CPU_H__
#define __TK_SYSDEF_DEPEND_CPU_H__


/* CPU Core-dependent definition (RXv2) */
#include "../core/rxv2/sysdef.h"

/* ------------------------------------------------------------------------ */
/*
 * Internal Memorie (Main RAM without saddr)  0x00000 - 0x0FFFF
 */
#define INTERNAL_RAM_TOP	0x00000
#define INTERNAL_RAM_END	0x10000


/*
 * Settable interval range (millisecond)
 */
#define MIN_TIMER_PERIOD	1
#define MAX_TIMER_PERIOD	50


/* ------------------------------------------------------------------------ */
/*
 * Number of Interrupt vectors
 */
#define N_INTVEC		256	/* Number of Interrupt vectors */

/*
 * The number of the implemented bit width for priority value fields.
 */
#define INTPRI_BITWIDTH		4


/* ------------------------------------------------------------------------ */
/*
 * Interrupt Priority Levels
 */
#define MAX_INT_PRI		(12)		/* Highest Ext. interrupt level */
#define TIM_INT_PRI		(10)

/*
 * Time-event handler interrupt level
 */
#define TIMER_INTLEVEL		(TIM_INT_PRI)


#endif /* __TK_SYSDEF_DEPEND_CPU_H__ */
