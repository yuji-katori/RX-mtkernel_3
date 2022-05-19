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
; *	System dependencies definition (AP-RX63N-0A depended)
; *	Included also from assembler program.
; */

.IF __SYS_SYSDEF_DEPEND_H__ != 1
__SYS_SYSDEF_DEPEND_H__		.EQU	(1)


;/* CPU-dependent definition (R5F563N) */
.INCLUDE sys/sysdepend/cpu/r5f563n/sysdef.inc

;/* ------------------------------------------------------------------------ */
;/*
; * Maximum value of Power-saving mode switching prohibition request.
; * Use in tk_set_pow API.
; */
;#define LOWPOW_LIMIT	0x7fff		/* Maximum number for disabling */

;/*
; * System Clock
; */
;#define ICLK			(96000000)	/* Instruction Clock (96MHz) */
;#define PCLK			(48000000)	/* Peripheral Clock  (48MHz) */

.ENDIF ; __TK_SYSDEF_DEPEND_H__ */
