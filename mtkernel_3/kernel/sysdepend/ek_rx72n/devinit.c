/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2023 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2025/2/9.
 *----------------------------------------------------------------------
 */
#include <sys/machine.h>
#ifdef EK_RX72N

/*
 *	devinit.c (EK-RX72N)
 *	Device-Dependent Initialization
 */

#include <sys/sysdef.h>
#include <tm/tmonitor.h>
#include "kernel.h"

#include "sysdepend.h"
#ifndef	NOUSE_SIIC
#include "dev_siic.h"
#endif	/* NOUSE_SIIC */

/* ------------------------------------------------------------------------ */

/*
 * Initialization before micro T-Kernel starts
 */

EXPORT ER knl_init_device( void )
{
	return E_OK;
}

/* ------------------------------------------------------------------------ */
/*
 * Start processing after T-Kernel starts
 *	Called from the initial task contexts.
 */
EXPORT ER knl_start_device( void )
{
#ifndef	NOUSE_SIIC
	siicDrvEntry( );			// Entry Simple I2C Driver
#endif	/* NOUSE_SIIC */
	return E_OK;
}

#if USE_SHUTDOWN
/* ------------------------------------------------------------------------ */
/*
 * System finalization
 *	Called just before system shutdown.
 *	Execute finalization that must be done before system shutdown.
 */
EXPORT ER knl_finish_device( void )
{
	return E_OK;
}

#endif /* USE_SHUTDOWN */

#endif /* EK_RX72N */
