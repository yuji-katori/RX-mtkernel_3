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
 *	System dependencies definition (TB-RX231 depended)
 *	Included also from assembler program.
 */

#ifndef __SYS_SYSDEF_DEPEND_H__
#define __SYS_SYSDEF_DEPEND_H__


/* CPU-dependent definition (R5F5231) */
#include "../cpu/r5f5231/sysdef.h"

/* ------------------------------------------------------------------------ */
/*
 * Maximum value of Power-saving mode switching prohibition request.
 * Use in tk_set_pow API.
 */
#define LOWPOW_LIMIT	0x7fff		/* Maximum number for disabling */

/*
 * System Clock
 */
#define ICLK			(54000000)	/* Instruction Clock (54MHz) */
#define PCLKA			(54000000)	/* Peripheral Clock  (54MHz) */
#define PCLKB			(27000000)	/* Peripheral Clock  (27MHz) */
#define PCLKD			(54000000)	/* Peripheral Clock  (54MHz) */

#endif /* __TK_SYSDEF_DEPEND_H__ */
