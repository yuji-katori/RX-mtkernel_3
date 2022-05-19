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
; *	config.inc
; *	User Configuration Definition
; */

.IF __TK_CONFIG__ != 1
__TK_CONFIG__	.EQU	(1)


;/*---------------------------------------------------------------------- */
;/* SYSCONF : micro T-Kernel system configuration
; */

CFN_SYSTEMAREA_TOP	.EQU	0	;/* 0: Use system default address */
CFN_SYSTEMAREA_END	.EQU	0	;/* 0: Use system default address */


;/*---------------------------------------------------------------------- */
; * Use RX core register Definition
; */
USE_FPU			.EQU	(1)	;/* use float,double,long double */
USE_DSP			.EQU	(0)	;/* use rmpab,rmpaw,rmpal */


;/*---------------------------------------------------------------------- */
;/* Stack size definition
; */
CFN_EXC_STACK_SIZE	.EQU	(1024)	;/* Exception stack size */


;/*---------------------------------------------------------------------- */
;/* System function selection
; *   1: Use function.  0: No use function.
; */
USE_IMALLOC		.EQU	(1)	;/* Use dynamic memory allocation */

.ENDIF ; __TK_CONFIG__
