/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	drv_lock.c
 *
 *	Driver Lock
 */

#include <tk/tkernel.h>

EXPORT void drv_lock(INT mode, W *lock)
{
W work;
	if( mode == TRUE )  {						// Lock Process
		work = TRUE;						// Set Lock Value
		while( __xchg( lock, &work ), work == TRUE )		// Wait Unlock
			tk_dly_tsk( 1 );				// Wait 1ms
	}
	else  {								// Unlock Process
		work = FALSE;						// Set Unlock Value
		__xchg(	lock, &work );					// Unlock
	}
}