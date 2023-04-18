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
 *	sys_msg.h (TB-RX65N)
 *	Hardware-Dependent System message
 */

#ifndef _SYSDEPEND_TARGET_SYSMSG_
#define _SYSDEPEND_TARGET_SYSMSG_

#include <tm/tmonitor.h>

#if (USE_SYSTEM_MESSAGE && USE_TMONITOR)
#ifdef CLANGSPEC
#define SYSTEM_MESSAGE(s)	tm_putstring((VB*)s)
#else
#define SYSTEM_MESSAGE(s)	tm_putstring((UB*)s)
#endif /* CLANGSPEC */
#else
#define SYSTEM_MESSAGE(s)
#endif /* USE_SYSTEM_MESSAGE && USE_TMONITOR */

#endif /* _SYSDEPEND_TARGET_SYSMSG_ */