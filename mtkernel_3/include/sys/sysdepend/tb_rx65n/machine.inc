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
; *	Machine type definition (TB-RX65N depended)
; */

.IF __SYS_SYSDEPEND_MACHINE_H__ != 1
__SYS_SYSDEPEND_MACHINE_H__	.EQU	(1)

;/*
; * [TYPE]_[CPU]	TARGET SYSTEM
; * CPU_xxxx		CPU type
; * CPU_CORE_xxx	CPU core type
; */

;/* ----- TB-RX65N (CPU: R5F565N**) definition ----- */
;#undef _TB_RX65N_

TB_RX65N	.EQU	(1)			;/* Target system : TB-RX65N */
CPU_R5F565N	.EQU	(1)			;/* Target CPU : RENESAS R5F565N */
CPU_CORE_RXV2	.EQU	(1)			;/* Target CPU-Core : RXv2 */

;#define TARGET_DIR		tb_rx65n			/* Sysdepend-Directory name */

;/*
; **** CPU-depeneded profile (R5F565N)
; */
;#include "../cpu/r5f565n/machine.h"


.ENDIF ; __SYS_SYSDEPEND_MACHINE_H__
