;/*
; *----------------------------------------------------------------------
; *    micro T-Kernel 3.00.00
; *
; *    Copyright (C) 2006-2023 by Ken Sakamura.
; *    This software is distributed under the T-License 2.2.
; *----------------------------------------------------------------------
; *
; *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
; *
; *----------------------------------------------------------------------
; */

;/*
; *	sysdef.inc
; *
; *	System dependencies definition (EK-RX72N depended)
; *	Included also from assembler program.
; */

.IF __SYS_SYSDEF_DEPEND_H__ != 1
__SYS_SYSDEF_DEPEND_H__		.EQU	(1)


;/* CPU-dependent definition (R5F572N) */
.INCLUDE sys/sysdepend/cpu/r5f572n/sysdef.inc

;/* ------------------------------------------------------------------------ */
;/*
; * Maximum value of Power-saving mode switching prohibition request.
; * Use in tk_set_pow API.
; */
;#define LOWPOW_LIMIT	0x7fff		/* Maximum number for disabling */

;/*
; * System Clock
; */
;#define ICLK			(240000000)	/* Instruction Clock (240MHz) */
;#define PCLKA			(120000000)	/* Peripheral Clock  (120MHz) */
;#define PCLKB			(60000000)	/* Peripheral Clock  (60MHz) */
;#define PCLKC			(60000000)	/* Peripheral Clock  (60MHz) */
;#define PCLKD			(60000000)	/* Peripheral Clock  (60MHz) */

.ENDIF ; __TK_SYSDEF_DEPEND_H__ */
