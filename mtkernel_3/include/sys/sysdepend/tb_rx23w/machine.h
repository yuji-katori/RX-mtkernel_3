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
 *	machine.h
 *
 *	Machine type definition (TB-RX23W depended)
 */

#ifndef __SYS_SYSDEPEND_MACHINE_H__
#define __SYS_SYSDEPEND_MACHINE_H__

/*
 * [TYPE]_[CPU]		TARGET SYSTEM
 * CPU_xxxx		CPU type
 * CPU_CORE_xxx		CPU core type
 */

/* ----- TB-RX23W (CPU: R5F523W**) definition ----- */
#undef _TB_RX23W_

#define TB_RX23W		1				/* Target system : TB-RX23W */
#define CPU_R5F523W		1				/* Target CPU : RENESAS R5F523W */
#define CPU_CORE_RXV2		1				/* Target CPU-Core : RXv2 */

#define TARGET_DIR		tb_rx23w			/* Sysdepend-Directory name */

/*
 **** CPU-depeneded profile (R5F523w)
 */
#include "../cpu/r5f523w/machine.h"


#endif /* __SYS_SYSDEPEND_MACHINE_H__ */
