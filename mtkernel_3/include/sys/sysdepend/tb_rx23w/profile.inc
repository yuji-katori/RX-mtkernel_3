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
; *	profile.h
; *
; *	Service Profile (TB-RX23W depended)
; */

.IF __SYS_DEPEND_PROFILE_H__ != 1
__SYS_DEPEND_PROFILE_H__	.EQU	(1)

TRUE		.EQU		(1)		;/* True */
FALSE		.EQU		(0)		;/* False */

;/*
; **** CPU-depeneded profile (R5F523W)
; */
;#include "../cpu/r5f523w/profile.h"

;/*
; **** Target-depeneded profile (TB-RX23W)
; */

;/*
; * Power management
; */
TK_SUPPORT_LOWPOWER	.EQU	FALSE		;/* Support of power management */


.ENDIF ;/* __SYS_DEPEND_PROFILE_H__ */
