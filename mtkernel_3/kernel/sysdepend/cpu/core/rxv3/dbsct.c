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
#include <sys/machine.h>
#ifdef CPU_CORE_RXV3
/*
 *	dbsct.c (RXv3)
 *	Section Initialize Table DTBL/BTBL
 */

#pragma unpack
#pragma section C C$DSEC
const struct {
	void *rom_s;	/* Start address of the initialized data section in ROM */
	void *rom_e;	/* End address of the initialized data section in ROM   */
	void *ram_s;	/* Start address of the initialized data section in RAM */
} _DTBL[] = {
	{ __sectop("D_8"), __secend("D_8"), __sectop("R_8") },
	{ __sectop("D"),   __secend("D"),   __sectop("R")   },
	{ __sectop("D_2"), __secend("D_2"), __sectop("R_2") },
	{ __sectop("D_1"), __secend("D_1"), __sectop("R_1") }
};
#pragma section C C$BSEC
const struct {
	void *b_s;	/* Start address of non-initialized data section */
	void *b_e;	/* End address of non-initialized data section */
} _BTBL[] = {
	{ __sectop("B_8"), __secend("B_8") },
	{ __sectop("B"),   __secend("B")   },
	{ __sectop("B_2"), __secend("B_2") },
	{ __sectop("B_1"), __secend("B_1") },
};
#pragma section
#pragma packoption

#endif	/* CPU_CORE_RXV3 */