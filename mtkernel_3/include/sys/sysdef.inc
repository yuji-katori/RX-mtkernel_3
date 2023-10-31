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
; *	System dependencies definition.
; *	Included also from assembler program.
; */

.IF __SYS_SYSDEF_H__ != 1
__SYS_SYSDEF_H__	.EQU	(1)

;/* System dependencies */
;#define SYSDEF_PATH_(a)		#a
;#define SYSDEF_PATH(a)		SYSDEF_PATH_(a)
;#define SYSDEF_SYSDEP()		SYSDEF_PATH(sysdepend/TARGET_DIR/sysdef.h)
;#include SYSDEF_SYSDEP()

.IF _AP_RX63N_ == 1
.INCLUDE sys/sysdepend/ap_rx63n/sysdef.inc
.ENDIF
.IF _GR_SAKURA_ == 1
.INCLUDE sys/sysdepend/gr_sakura/sysdef.inc
.ENDIF
.IF _AP_RX65N_ == 1
.INCLUDE sys/sysdepend/ap_rx65n/sysdef.inc
.ENDIF
.IF _AP_RX72N_ == 1
.INCLUDE sys/sysdepend/ap_rx72n/sysdef.inc
.ENDIF
.IF _TB_RX231_ == 1
.INCLUDE sys/sysdepend/tb_rx231/sysdef.inc
.ENDIF
.IF _TB_RX23W_ == 1
.INCLUDE sys/sysdepend/tb_rx23w/sysdef.inc
.ENDIF
.IF _TB_RX65N_ == 1
.INCLUDE sys/sysdepend/tb_rx65N/sysdef.inc
.ENDIF
.IF _TB_RX660_ == 1
.INCLUDE sys/sysdepend/tb_rx660/sysdef.inc
.ENDIF
.IF _TB_RX66N_ == 1
.INCLUDE sys/sysdepend/tb_rx66n/sysdef.inc
.ENDIF
.IF _EK_RX72N_ == 1
.INCLUDE sys/sysdepend/ek_rx72n/sysdef.inc
.ENDIF

.ENDIF ; __SYS_SYSDEF_H__
