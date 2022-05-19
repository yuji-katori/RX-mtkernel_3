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
 *	syslib.h
 *
 *	micro T-Kernel System Library  (RXv1 core depended)
 */

#ifndef __TK_SYSLIB_DEPEND_CORE_H__
#define __TK_SYSLIB_DEPEND_CORE_H__

#include <tk/errno.h>
#include <sys/sysdef.h>

/*----------------------------------------------------------------------*/
/*
 * CPU interrupt control for RXv1.
 *	'intsts' is the negate value of PSW register in CPU
 *	disint()  Set PSW.IE = 0 and return the negate original PSW to the return value.
 *	enaint()  Set PSW.IE = ~intsts.IE. Do not change except for PSW.IE.
 *		  Return the negate original PSW to the return value.
 */

IMPORT UINT disint( void );
IMPORT UINT enaint( UINT intsts );

#define DI(intsts)		( (intsts) = disint() )
#define EI(intsts)		( enaint(intsts) )
#define isDI(intsts)		( (intsts) != 0 )

#define INTLEVEL_DI		(MAX_INT_PRI)
#define INTLEVEL_EI		(0)

/*
 * Set Interrupt Mask Level in CPU
 */
IMPORT void SetCpuIntLevel( INT level );

/*
 * Get Interrupt Mask Level in CPU
 */
IMPORT INT GetCpuIntLevel( void );

/*
 * Convert to interrupt definition number
 *
 * For backward compatibility.
 * 	INTVEC has been obsoleted since micro T-Kernel 2.0.
 */
#define DINTNO(intvec)	(intvec)

/* ------------------------------------------------------------------------ */
/*
 * I/O port access
 *	for memory mapped I/O
 */
Inline void out_w( INT port, UW data )
{
	*(_UW*)port = data;
}
Inline void out_h( INT port, UH data )
{
	*(_UH*)port = data;
}
Inline void out_b( INT port, UB data )
{
	*(_UB*)port = data;
}

Inline UW in_w( INT port )
{
	return *(_UW*)port;
}
Inline UH in_h( INT port )
{
	return *(_UH*)port;
}
Inline UB in_b( INT port )
{
	return *(_UB*)port;
}

#endif /* __TK_SYSLIB_DEPEND_CORE_H__ */
