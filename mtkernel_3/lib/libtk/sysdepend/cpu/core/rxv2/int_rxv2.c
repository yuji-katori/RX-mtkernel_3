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

#include <sys/machine.h>
#ifdef CPU_CORE_RXV2

/*
 *	int.c
 *
 *	Interrupt controller (RXv2)
 */

#include <tk/tkernel.h>

/*----------------------------------------------------------------------*/
/*
 * CPU Interrupt Control for RXv2.
 *
 */

/* 
 * Disable interrupt 
 */
EXPORT UINT disint(void)
{
UINT ret = __get_ipl( );
	__set_ipl( MAX_INT_PRI );
	return ret;
}


/*
 * Enable interrupt
 */
UINT enaint( UINT intsts )
{
UINT ret = __get_ipl( );
	__set_ipl( intsts );
	return ret;
}


#endif /* CPU_CORE_RXV2 */
