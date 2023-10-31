/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2023 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 */

/*
 *	machine.h
 *
 *	Machine type definition (EK-RX72N depended)
 */

#ifndef __SYS_SYSDEPEND_MACHINE_H__
#define __SYS_SYSDEPEND_MACHINE_H__

/*
 * [TYPE]_[CPU]		TARGET SYSTEM
 * CPU_xxxx		CPU type
 * CPU_CORE_xxx		CPU core type
 */

/* ----- EK-RX72N-0A (CPU: R5F572N**) definition ----- */
#undef _EK_RX72N_

#define EK_RX72N		1				/* Target system : EK-RX72N */
#define CPU_R5F572N		1				/* Target CPU : RENESAS R5F572N */
#define CPU_CORE_RXV3		1				/* Target CPU-Core : RXv3 */

#define TARGET_DIR		ek_rx72n			/* Sysdepend-Directory name */

/*
 **** CPU-depeneded profile (R5F572N)
 */
#include "../cpu/r5f572n/machine.h"


#endif /* __SYS_SYSDEPEND_MACHINE_H__ */
