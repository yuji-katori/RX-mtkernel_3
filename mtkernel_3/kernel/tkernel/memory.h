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
 *	memory.h
 *	In-kernel dynamic memory management
 */

#ifndef _MEMORY_H_
#define _MEMORY_H_

#include "limits.h"

/*
 * Memory allocation management information
 *
 *  Order of members must not be changed because members are used
 *  with casting from MPLCB.
 */
typedef struct {
	SZ		memsz;

	/* AreaQue for connecting each area where reserved pages are
	   divided Sort in ascending order of addresses in a page.
	   Do not sort between pages. */
	QUEUE		areaque;
	/* FreeQue for connecting unused area in reserved pages
	   Sort from small to large free spaces. */
	QUEUE		freeque;
} IMACB;

/*
 * Compensation for aligning "&areaque" position to 2 bytes border
 */
#define AlignIMACB(imacb)	( (IMACB*)((size_t)(imacb) & ~((size_t)0x1)) )

/*
 * Minimum unit of subdivision
 *	The lower 1 bit of address is always 0
 *	because memory is allocated by ROUNDSZ.
 *	AreaQue uses the lower 1 bit for flag.
 */
#define ROUNDSZ		( sizeof(QUEUE) )	/* 8 bytes */
#define ROUND(sz)	( ((sz) + (ROUNDSZ-1)) & ~(ROUNDSZ-1) )

/* Minimum fragment size */
#define MIN_FRAGMENT	( sizeof(QUEUE) * 2 )

/*
 * Maximum allocatable size (to check for parameter)
 */
#define	MAX_ALLOCATE	( INT_MAX & ~(ROUNDSZ-1) )

/*
 * Adjusting the size which can be allocated 
 */
Inline SZ roundSize( SZ sz )
{
	if ( sz < MIN_FRAGMENT ) {
		sz = MIN_FRAGMENT;
	}
	return (sz + (ROUNDSZ-1)) & ~(ROUNDSZ-1);
}


/*
 * Flag that uses the lower bits of AreaQue's 'prev'.
 */
#define AREA_USE	((size_t)1)	/* In-use */
#define AREA_MASK	((size_t)1)

#define setAreaFlag(q, f)   ( (q)->prev = (QUEUE*)((size_t)(q)->prev |  (f)) )
#define clrAreaFlag(q, f)   ( (q)->prev = (QUEUE*)((size_t)(q)->prev & ~(f)) )
#define chkAreaFlag(q, f)   ( (size_t)(q)->prev & (f) != 0 )

#define Mask(x)		( (QUEUE*)((size_t)(x) & ~AREA_MASK) )
#define Assign(x, y)	( (x) = (QUEUE*)(((size_t)(x) & AREA_MASK) | (size_t)(y)) )
/*
 * Area size
 */
#define AreaSize(aq)	( (VB*)(aq)->next - (VB*)((aq) + 1) )
#define FreeSize(fq)	( (SZ)((fq) + 1)->prev )


IMPORT QUEUE* knl_searchFreeArea( IMACB *imacb, SZ blksz );
IMPORT void knl_appendFreeArea( IMACB *imacb, QUEUE *aq );
IMPORT void knl_removeFreeQue( QUEUE *fq );
IMPORT void knl_insertAreaQue( QUEUE *que, QUEUE *ent );
IMPORT void knl_removeAreaQue( QUEUE *aq );

IMPORT IMACB *knl_imacb;

#endif /* _MEMORY_H_ */
