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
 *	profile.h
 *
 *	Service Profile (TB-RX231 depended)
 */

#ifndef __SYS_DEPEND_PROFILE_H__
#define __SYS_DEPEND_PROFILE_H__

/*
 **** CPU-depeneded profile (R5F5231)
 */
#include "../cpu/r5f5231/profile.h"

/*
 **** Target-depeneded profile (TB-RX231)
 */

/*
 * Power management
 */
#define TK_SUPPORT_LOWPOWER	FALSE		/* Support of power management */



#endif /* __SYS_DEPEND_PROFILE_H__ */
