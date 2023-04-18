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
; *	Machine type definition (TB-RX231 depended)
; */

.IF __SYS_SYSDEPEND_MACHINE_H__ != 1
__SYS_SYSDEPEND_MACHINE_H__	.EQU	(1)

;/*
; * [TYPE]_[CPU]	TARGET SYSTEM
; * CPU_xxxx		CPU type
; * CPU_CORE_xxx	CPU core type
; */

;/* ----- TB-RX231 (CPU: R5F5231**) definition ----- */
;#undef _TB_RX231_

TB_RX231	.EQU	(1)			;/* Target system : TB-RX231 */
CPU_R5F5231	.EQU	(1)			;/* Target CPU : RENESAS R5F5231 */
CPU_CORE_RXV2	.EQU	(1)			;/* Target CPU-Core : RXv2 */

;#define TARGET_DIR		tb_rx231			/* Sysdepend-Directory name */

;/*
; **** CPU-depeneded profile (R5F5231)
; */
;#include "../cpu/r5f5231/machine.h"


.ENDIF ; __SYS_SYSDEPEND_MACHINE_H__
