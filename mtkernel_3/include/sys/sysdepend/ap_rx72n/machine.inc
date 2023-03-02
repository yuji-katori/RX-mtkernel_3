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
; *	Machine type definition (AP-RX72N-0A depended)
; */

.IF __SYS_SYSDEPEND_MACHINE_H__ != 1
__SYS_SYSDEPEND_MACHINE_H__	.EQU	(1)

;/*
; * [TYPE]_[CPU]	TARGET SYSTEM
; * CPU_xxxx		CPU type
; * CPU_CORE_xxx	CPU core type
; */

;/* ----- AP-RX72N-0A (CPU: R5F572N**) definition ----- */
;#undef _AP_RX72N_

AP_RX72N	.EQU	(1)			;/* Target system : AP-RX72N-0A */
CPU_R5F572N	.EQU	(1)			;/* Target CPU : RENESAS R5F572N */
CPU_CORE_RXV3	.EQU	(1)			;/* Target CPU-Core : RXv3 */

;#define TARGET_DIR		ap_rx72n			/* Sysdepend-Directory name */

;/*
; **** CPU-depeneded profile (R5F572N)
; */
.INCLUDE sys/sysdepend/cpu/r5f572n/machine.inc


.ENDIF ; __SYS_SYSDEPEND_MACHINE_H__
