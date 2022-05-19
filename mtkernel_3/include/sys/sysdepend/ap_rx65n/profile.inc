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
; *	Service Profile (AP-RX65N-0A depended)
; */

.IF __SYS_DEPEND_PROFILE_H__ != 1
__SYS_DEPEND_PROFILE_H__	.EQU	(1)

TRUE		.EQU		(1)		;/* True */
FALSE		.EQU		(0)		;/* False */

;/*
; **** CPU-depeneded profile (R5F565N)
; */
;#include "../cpu/r5f565n/profile.h"

;/*
; **** Target-depeneded profile (AP-RX65N-0A)
; */

;/*
; * Power management
; */
TK_SUPPORT_LOWPOWER	.EQU	FALSE		;/* Support of power management */


.ENDIF ;/* __SYS_DEPEND_PROFILE_H__ */
