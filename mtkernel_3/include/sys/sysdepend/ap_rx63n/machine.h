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
 *	Machine type definition (AP-RX63N-0A depended)
 */

#ifndef __SYS_SYSDEPEND_MACHINE_H__
#define __SYS_SYSDEPEND_MACHINE_H__

/*
 * [TYPE]_[CPU]		TARGET SYSTEM
 * CPU_xxxx		CPU type
 * CPU_CORE_xxx		CPU core type
 */

/* ----- AP-RX63N-0A (CPU: R5F563N) definition ----- */
#undef _AP_RX63N_

#define AP_RX63N		1				/* Target system : AP-RX63N-0A */
#define CPU_R5F563N		1				/* Target CPU : RENESAS R5F563N */
#define CPU_CORE_RXV1		1				/* Target CPU-Core : RXv1 */

#define TARGET_DIR		ap_rx63n			/* Sysdepend-Directory name */

/*
 **** CPU-depeneded profile (R5F563N)
 */
#include "../cpu/r5f563n/machine.h"


#endif /* __SYS_SYSDEPEND_MACHINE_H__ */
