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
; *	knldef.inc
; *	micro T-Kernel system definition form Configulation
; */

.IF _SYS_KNLDEF_H_ != 1
_SYS_KNLDEF_H_	.EQU	(1)

;/*---------------------------------------------------------------------- */
;/* Stack size definition
; */
EXC_STACK_SIZE	.EQU	CFN_EXC_STACK_SIZE

.ENDIF ; _SYS_KNLDEF_H_
