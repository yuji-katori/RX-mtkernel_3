/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	groupbl2.c
 *
 *	Renesas GroupBL2 Interrupt Handler
 */

#include <tk/tkernel.h>
#include "iodefine.h"

EXPORT void (*GroupBL2Table[32])(UINT dintno);		// Handler Address Table

EXPORT void GroupBL2Handler(UINT dintno)
{
INT i;
UW intreqflg;
	intreqflg = ICU.GRPBL2.LONG;			// Read Interruupt Reuest Flag
	for( i=0 ; !( intreqflg & 0xFF ) ; i+=8 )	// Request in Lower 8 bit ?
		intreqflg >>= 8;			// Shift Interrupt Request Flag
	for( ; intreqflg ; i++, intreqflg >>= 1 )	// Search Interrupt Factor
		if( intreqflg & 1 )			// Check Interrupt Request Flag
			GroupBL2Table[i]( dintno );	// Call Interrupt Handler
}