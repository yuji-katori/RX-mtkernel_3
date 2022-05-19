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
 *	memory.c
 *	In-kernel dynamic memory management
 */

#include "kernel.h"
#include "memory.h"

/*
 * FreeQue search
 *	Search the free area whose size is equal to 'blksz',
 *	or larger than
 *      'blksz' but closest.
 *	If it does not exist, return '&imacb->freeque'.
 */
EXPORT QUEUE* knl_searchFreeArea( IMACB *imacb, SZ blksz )
{
	QUEUE	*q = &imacb->freeque;

	/* For area whose memory pool size is less than 1/4,
	   search from smaller size.
	   Otherwise, search from larger size. */
	if ( blksz > (imacb->memsz >> 2) ) {
		/* Search from larger size. */
		SZ fsz = 0;
		while ( (q = q->prev) != &imacb->freeque ) {
			fsz = FreeSize(q);
			if ( fsz <= blksz ) {
				return ( fsz < blksz )? q->next: q;
			}
		}
		return ( fsz >= blksz )? q->next: q;
	} else {
		/* Search from smaller size. */
		while ( (q = q->next) != &imacb->freeque ) {
			if ( FreeSize(q) >= blksz ) {
				break;
			}
		}
		return q;
	}
}


/*
 * Registration of free area on FreeQue
 *	FreeQue is composed of 2 types: Queue that links the
 *	different size of areas by size and queue that links the
 *	same size of areas.
 *
 *	freeque
 *	|
 *	|   +-----------------------+	    +-----------------------+
 *	|   | AreaQue		    |	    | AreaQue		    |
 *	|   +-----------------------+	    +-----------------------+
 *	*---> FreeQue Size order    |	    | EmptyQue		    |
 *	|   | FreeQue Same size   --------->| FreeQue Same size   ----->
 *	|   |			    |	    |			    |
 *	|   |			    |	    |			    |
 *	|   +-----------------------+	    +-----------------------+
 *	|   | AreaQue		    |	    | AreaQue		    |
 *	v   +-----------------------+	    +-----------------------+
 */
EXPORT void knl_appendFreeArea( IMACB *imacb, QUEUE *aq )
{
	QUEUE	*fq;
	SZ	size = AreaSize(aq);

	/* Registration position search */
	/*  Search the free area whose size is equal to 'blksz',
	 *  or larger than 'blksz' but closest.
	 *  If it does not exist, return '&imacb->freeque'.
	 */
	fq = knl_searchFreeArea(imacb, size);

	/* Register */
	clrAreaFlag(aq, AREA_USE);
	if ( fq != &imacb->freeque && FreeSize(fq) == size ) {
		/* FreeQue Same size */
		(aq + 2)->next = (fq + 1)->next;
		(fq + 1)->next = aq + 2;
		(aq + 2)->prev = fq + 1;
		if( (aq + 2)->next != NULL ) {
			(aq + 2)->next->prev = aq + 2;
		}
		(aq + 1)->next = NULL;
	} else {
		/* FreeQue Size order */
		QueInsert(aq + 1, fq);
		(aq + 2)->next = NULL;
		(aq + 2)->prev = (QUEUE*)size;
	}
}

/*
 * Delete from FreeQue
 */
EXPORT void knl_removeFreeQue( QUEUE *fq )
{
	if ( fq->next == NULL ) {	/* FreeQue Same size */
		(fq + 1)->prev->next = (fq + 1)->next;
		if ( (fq + 1)->next != NULL ) {
			(fq + 1)->next->prev = (fq + 1)->prev;
		}
	} else {			/* FreeQue Size order */
		if ( (fq + 1)->next != NULL ) {		/* having FreeQue Same size */
			QueInsert((fq + 1)->next - 1, fq);
			(fq + 1)->next->prev = (fq + 1)->prev;
		}
		QueRemove(fq);
	}
}

/*
 * Register area
 *	Insert 'ent' just after 'que.'
 */
EXPORT void knl_insertAreaQue( QUEUE *que, QUEUE *ent )
{
	ent->prev = que;
	ent->next = que->next;
	Assign(que->next->prev, ent);
	que->next = ent;
}

/*
 * Delete area
 */
EXPORT void knl_removeAreaQue( QUEUE *aq )
{
	Mask(aq->prev)->next = aq->next;
	Assign(aq->next->prev, Mask(aq->prev));
}

/* ------------------------------------------------------------------------ */

#if USE_IMALLOC
/* ------------------------------------------------------------------------ */

Noinit(EXPORT IMACB *knl_imacb);

/* ------------------------------------------------------------------------ */

/*
 * Get memory 
 */
EXPORT void* knl_Imalloc( SZ size )
{
	QUEUE	*q, *aq, *aq2;
	UINT	imask;

	/* If it is smaller than the minimum fragment size,
	   allocate the minimum size to it. */
	if( size <= 0 ) {
		return (void *)NULL;
	} else 	if ( size < MIN_FRAGMENT ) {
		size = MIN_FRAGMENT;
	} else {
		size = ROUND(size);
	}

	DI(imask);  /* Exclusive control by interrupt disable */

	/* Search FreeQue */
	q = knl_searchFreeArea(knl_imacb, size);
	if ( q == &(knl_imacb->freeque) ) {
		q = NULL; /* Insufficient memory */
		goto err_ret;
	}

	/* There is free area: Split from FreeQue once */
	knl_removeFreeQue(q);

	aq = q - 1;

	/* If there are fragments smaller than the minimum fragment size,
	   allocate them also */
	if ( FreeSize(q) - size >= MIN_FRAGMENT + sizeof(QUEUE) ) {

		/* Divide area into 2 */
		aq2 = (QUEUE*)((VB*)(aq + 1) + size);
		knl_insertAreaQue(aq, aq2);

		/* Register remaining area to FreeQue */
		knl_appendFreeArea(knl_imacb, aq2);
	}
	setAreaFlag(aq, AREA_USE);

err_ret:
	EI(imask);

	return (void *)q;
}

/*
 * Get memory
 */
EXPORT void* knl_Icalloc( SZ nmemb, SZ size )
{
	SZ	sz = nmemb * size;
	void	*mem;

	mem = knl_Imalloc(sz);
	if ( mem == NULL ) {
		return NULL;
	}

	knl_memset(mem, 0, sz);

	return mem;
}

/*
 * Free memory
 *	It may be called during interrupt disable. In this case, need to wait
 *	 until interrupt is enabled and until free.
 */
EXPORT void  knl_Ifree( void *ptr )
{
	QUEUE	*aq;
	UINT	imask;

	DI(imask);  /* Exclusive control by interrupt disable */

	aq = (QUEUE*)ptr - 1;
	clrAreaFlag(aq, AREA_USE);

	if ( !chkAreaFlag(aq->next, AREA_USE) ) {
		/* Merge with free area in after location */
		knl_removeFreeQue(aq->next + 1);
		knl_removeAreaQue(aq->next);
	}

	if ( !chkAreaFlag(aq->prev, AREA_USE) ) {
		/* Merge with free area in front location */
		aq = aq->prev;
		knl_removeFreeQue(aq + 1);
		knl_removeAreaQue(aq->next);
	}

	knl_appendFreeArea(knl_imacb, aq);

	EI(imask);
}

/* ------------------------------------------------------------------------ */

/*
 * IMACB Initialization 
 */
LOCAL void initIMACB( void )
{
	QueInit(&(knl_imacb->areaque));
	QueInit(&(knl_imacb->freeque));
}

/*
 * Imalloc initial setting 
 */
EXPORT ER knl_init_Imalloc( void )
{
	QUEUE	*top, *end;

	/* Align top with 4 byte unit alignment for IMACB */
	knl_lowmem_top = (void *)(((size_t)knl_lowmem_top + 3) & ~(size_t)3);
	knl_imacb = (IMACB*)knl_lowmem_top;
	knl_lowmem_top = (void *)((size_t)knl_lowmem_top + sizeof(IMACB));

	/* Align top with 8 byte unit alignment */
	knl_lowmem_top = (void *)(((size_t)knl_lowmem_top + 7) & ~(size_t)7);
	top = (QUEUE*)knl_lowmem_top;
	knl_imacb->memsz = (SZ)((size_t)knl_lowmem_limit - (size_t)knl_lowmem_top - sizeof(QUEUE)*2);

	knl_lowmem_top = knl_lowmem_limit;  /* Update memory free space */

	initIMACB();

	/* Register on AreaQue */
	end = (QUEUE*)((VB*)top + knl_imacb->memsz) + 1;
	knl_insertAreaQue(&knl_imacb->areaque, end);
	knl_insertAreaQue(&knl_imacb->areaque, top);
	setAreaFlag(end, AREA_USE);
	setAreaFlag(&knl_imacb->areaque, AREA_USE);

	knl_appendFreeArea(knl_imacb, top);

	return E_OK;
}

#endif /* USE_IMALLOC */
