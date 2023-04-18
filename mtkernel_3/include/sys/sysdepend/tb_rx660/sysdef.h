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
 *	System dependencies definition (TB-RX660 depended)
 *	Included also from assembler program.
 */

#ifndef __SYS_SYSDEF_DEPEND_H__
#define __SYS_SYSDEF_DEPEND_H__


/* CPU-dependent definition (R5F5660) */
#include "../cpu/r5f5660/sysdef.h"

/* ------------------------------------------------------------------------ */
/*
 * Maximum value of Power-saving mode switching prohibition request.
 * Use in tk_set_pow API.
 */
#define LOWPOW_LIMIT	0x7fff		/* Maximum number for disabling */

/*
 * System Clock
 */
#define ICLK			(120000000)	/* Instruction Clock (120MHz) */
#define PCLKA			(120000000)	/* Peripheral Clock  (120MHz) */
#define PCLKB			(60000000)	/* Peripheral Clock  (60MHz) */
#define PCLKD			(60000000)	/* Peripheral Clock  (60MHz) */

#endif /* __TK_SYSDEF_DEPEND_H__ */
