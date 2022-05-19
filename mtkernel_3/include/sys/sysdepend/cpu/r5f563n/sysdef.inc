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
; *	System dependencies definition (R5F563N depended)
; *	Included also from assembler program.
; */

.IF __TK_SYSDEF_DEPEND_CPU_H__ != 1
__TK_SYSDEF_DEPEND_CPU_H__	.EQU	(1)


;/* CPU Core-dependent definition (RXv1) */
;#include "../core/rxv1/sysdef.h"

;/* ------------------------------------------------------------------------ */
;/*
; * Internal Memorie (Main RAM without saddr)  0x00000 - 0x1FFFF
; */
;#define INTERNAL_RAM_TOP	0x00000
INTERNAL_RAM_END	.EQU	20000H


;/*
; * Settable interval range (millisecond)
; */
;#define MIN_TIMER_PERIOD	1
;#define MAX_TIMER_PERIOD	50


;/* ------------------------------------------------------------------------ */
;/*
; * Number of Interrupt vectors
; */
;#define N_INTVEC		256	/* Number of Interrupt vectors */

;/*
; * The number of the implemented bit width for priority value fields.
; */
;#define INTPRI_BITWIDTH	4


;/* ------------------------------------------------------------------------ */
;/*
; * Interrupt Priority Levels
; */
MAX_INT_PRI		.EQU	(12)		;/* Highest Ext. interrupt level */
TIM_INT_PRI		.EQU	(10)

;/*
; * Time-event handler interrupt level
; */
;TIMER_INTLEVEL		.EQU	(TIM_INT_PRI)


.ENDIF ; __TK_SYSDEF_DEPEND_CPU_H__
