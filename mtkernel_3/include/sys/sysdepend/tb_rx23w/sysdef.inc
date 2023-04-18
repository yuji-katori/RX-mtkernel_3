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
; *	sysdef.inc
; *
; *	System dependencies definition (TB-RX23W depended)
; *	Included also from assembler program.
; */

.IF __SYS_SYSDEF_DEPEND_H__ != 1
__SYS_SYSDEF_DEPEND_H__		.EQU	(1)


;/* CPU-dependent definition (R5F523W) */
.INCLUDE sys/sysdepend/cpu/r5f523w/sysdef.inc

;/* ------------------------------------------------------------------------ */
;/*
; * Maximum value of Power-saving mode switching prohibition request.
; * Use in tk_set_pow API.
; */
;#define LOWPOW_LIMIT	0x7fff		/* Maximum number for disabling */

;/*
; * System Clock
; */
;#define ICLK			(54000000)	/* Instruction Clock (54MHz) */
;#define PCLKA			(54000000)	/* Peripheral Clock  (54MHz) */
;#define PCLKB			(27000000)	/* Peripheral Clock  (27MHz) */
;#define PCLKD			(54000000)	/* Peripheral Clock  (54MHz) */

.ENDIF ; __TK_SYSDEF_DEPEND_H__ */
