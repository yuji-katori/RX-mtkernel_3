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
 *	Machine type definition (TB-RX65N depended)
 */

#ifndef __SYS_SYSDEPEND_MACHINE_H__
#define __SYS_SYSDEPEND_MACHINE_H__

/*
 * [TYPE]_[CPU]		TARGET SYSTEM
 * CPU_xxxx		CPU type
 * CPU_CORE_xxx		CPU core type
 */

/* ----- TB-RX65N (CPU: R5F565N**) definition ----- */
#undef _TB_RX65N_

#define TB_RX65N		1				/* Target system : TB-RX65N */
#define CPU_R5F565N		1				/* Target CPU : RENESAS R5F565N */
#define CPU_CORE_RXV2		1				/* Target CPU-Core : RXv2 */

#define TARGET_DIR		tb_rx65n			/* Sysdepend-Directory name */

/*
 **** CPU-depeneded profile (R5F565N)
 */
#include "../cpu/r5f565n/machine.h"


#endif /* __SYS_SYSDEPEND_MACHINE_H__ */
