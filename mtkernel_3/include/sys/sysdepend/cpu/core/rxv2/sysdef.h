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
 *	sysdef_depend.h
 *
 *	System dependencies definition (RXv2 core depended)
 *	Included also from assembler program.
 */

#ifndef __SYS_SYSDEF_DEPEND_CORE_H__
#define __SYS_SYSDEF_DEPEND_CORE_H__

/* ------------------------------------------------------------------------ */
/*
 * size_t, prtdiff_t
 *
 */
#include <stddef.h>

/* ------------------------------------------------------------------------ */
/*
 * Definition of minimum system stack size
 *	Minimum system stack size when setting the system stack size
 *	per task by 'tk_cre_tsk().'
 *  this size must be larger than the size of SStackFrame
 */
#define MIN_SYS_STACK_SIZE	68+USE_FPU*4+USE_DSP*24

/*
 * Default task system stack 
 */

#define DEFAULT_SYS_STKSZ	0

/* ------------------------------------------------------------------------ */

#endif /* __SYS_SYSDEF_DEPEND_CORE_H__ */
