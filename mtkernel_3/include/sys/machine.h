/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2019 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 */

/*
 *	machine.h
 *	Machine type definition 
 */

#ifndef __SYS_MACHINE_H__
#define __SYS_MACHINE_H__

/* ===== System dependencies definitions ================================ */
#include <config.h>

#ifdef _AP_RX63N_
#include "sysdepend/ap_rx63n/machine.h"
#endif
#ifdef _GR_SAKURA_
#include "sysdepend/gr_sakura/machine.h"
#endif
#ifdef _AP_RX65N_
#include "sysdepend/ap_rx65n/machine.h"
#endif
#ifdef _AP_RX72N_
#include "sysdepend/ap_rx72n/machine.h"
#endif
#ifdef _TB_RX231_
#include "sysdepend/tb_rx231/machine.h"
#endif
#ifdef _TB_RX23W_
#include "sysdepend/tb_rx23w/machine.h"
#endif
#ifdef _TB_RX65N_
#include "sysdepend/tb_rx65n/machine.h"
#endif
#ifdef _TB_RX660_
#include "sysdepend/tb_rx660/machine.h"
#endif
#ifdef _TB_RX66N_
#include "sysdepend/tb_rx66n/machine.h"
#endif
#ifdef _EK_RX72N_
#include "sysdepend/ek_rx72n/machine.h"
#endif

/* ===== C compiler dependencies definitions ============================= */

#ifdef __CCRX__

#define Csym(sym) sym
#define Inline static __inline
#define Noinit(decl) decl

#endif /* __CCRX__ */

#endif /* __SYS_MACHINE_H__ */
