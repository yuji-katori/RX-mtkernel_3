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
 *	Machine type definition (RXv2 core depended)
 */

#ifndef __SYS_MACHINE_CORE_H__
#define __SYS_MACHINE_CORE_H__

/*
 * CPU_xxxx		CPU type
 * ALLOW_MISALIGN	1 if access to misalignment data is allowed 
 * BIGENDIAN		1 if big endian 
 */

/* ----- RXv2 definition ----- */

#define CPU_RX_V2		1
#define ALLOW_MISALIGN		0
#define INT_BITWIDTH		32

/*
 * Endianness
 */
#ifdef __LIT
#define BIGENDIAN		0	/* Default (Little Endian) */
#else
#define BIGENDIAN		1	/* Big Endian */
#endif

#endif /* __SYS_MACHINE_CORE_H__ */
