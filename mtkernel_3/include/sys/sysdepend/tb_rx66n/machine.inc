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
; *	Machine type definition (TB-RX66N depended)
; */

.IF __SYS_SYSDEPEND_MACHINE_H__ != 1
__SYS_SYSDEPEND_MACHINE_H__	.EQU	(1)

;/*
; * [TYPE]_[CPU]	TARGET SYSTEM
; * CPU_xxxx		CPU type
; * CPU_CORE_xxx	CPU core type
; */

;/* ----- TB-RX66N (CPU: R5F566N**) definition ----- */
;#undef _TB_RX66N_

TB_RX66N	.EQU	(1)			;/* Target system : TB-RX66N */
CPU_R5F566N	.EQU	(1)			;/* Target CPU : RENESAS R5F566N */
CPU_CORE_RXV3	.EQU	(1)			;/* Target CPU-Core : RXv3 */

;#define TARGET_DIR		tb_rx66n			/* Sysdepend-Directory name */

;/*
; **** CPU-depeneded profile (R5F566N)
; */
.INCLUDE sys/sysdepend/cpu/r5f566n/machine.inc


.ENDIF ; __SYS_SYSDEPEND_MACHINE_H__
