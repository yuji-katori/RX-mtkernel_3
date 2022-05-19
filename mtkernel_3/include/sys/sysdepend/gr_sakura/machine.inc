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
; *	machine.inc
; *
; *	Machine type definition (GR-SAKURA depended)
; */

.IF __SYS_SYSDEPEND_MACHINE_H__ != 1
__SYS_SYSDEPEND_MACHINE_H__	.EQU	(1)

;/*
; * [TYPE]_[CPU]	TARGET SYSTEM
; * CPU_xxxx		CPU type
; * CPU_CORE_xxx	CPU core type
; */

;/* ----- GR-SAKURA (CPU: R5F563N**) definition ----- */
;#undef _GR-SAKURA_

GR_SAKURA	.EQU	(1)			;/* Target system : GR-SAKURA RX63N */
CPU_R5F563N	.EQU	(1)			;/* Target CPU : RENESAS R5F563N */
CPU_CORE_RXV1	.EQU	(1)			;/* Target CPU-Core : RXv1 */

;#define TARGET_DIR		gr_sakura			/* Sysdepend-Directory name */

;/*
; **** CPU-depeneded profile (R5F563N)
; */
;#include "../cpu/r5f563n/machine.h"


.ENDIF ; __SYS_SYSDEPEND_MACHINE_H__
