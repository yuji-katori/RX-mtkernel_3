;/*
; *----------------------------------------------------------------------
; *    micro T-Kernel 3.00.00
; *
; *    Copyright (C) 2006-2019 by Ken Sakamura.
; *    This software is distributed under the T-License 2.1.
; *----------------------------------------------------------------------
; *
; *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
; *
; *----------------------------------------------------------------------
; */

;/*
; *	machine_depend.h
; *
; *	Machine type definition (R5F5660 depended)
; */

.IF __SYS_SYSDEPEND_MACHINE_CPU_H__ != 1
__SYS_SYSDEPEND_MACHINE_CPU_H__	.EQU	(1)

CPU_RXV3_REGBANK	.EQU	(1)
CPU_RXV3_DFPU		.EQU	(0)

;/*
; **** CPU core-depeneded profile (RXv3)
; */

;#include "../core/rxv3/machine.h"


.ENDIF ;/* __SYS_SYSDEPEND_MACHINE_CPU_H__ */
