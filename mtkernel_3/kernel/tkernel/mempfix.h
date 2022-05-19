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
 *	mempfix.h
 *	Fixed Size Memory Pool
 */

#ifndef _MEMPFIX_H_
#define _MEMPFIX_H_

/*
 * Fixed size memory pool control block
 */
typedef struct free_list {
	struct free_list *next;
} FREEL;

typedef struct fix_memorypool_control_block {
	QUEUE	wait_queue;	/* Memory pool wait queue */
	ID	mpfid;		/* Fixed size memory pool ID */
	void	*exinf;		/* Extended information */
	ATR	mpfatr;		/* Memory pool attribute */
	SZ	mpfcnt;		/* Number of blocks in whole memory pool */
	SZ	blfsz;		/* Fixed size memory block size */
	SZ	mpfsz;		/* Whole memory pool size */
	SZ	frbcnt;		/* Number of blocks in free area */
	void	*mempool;	/* Top address of memory pool */
	void	*unused;		/* Top address of unused area */
	FREEL	*freelist;	/* Free block list */
	OBJLOCK	lock;		/* Lock for object exclusive access */
#if USE_OBJECT_NAME
#ifdef CLANGSPEC
	VB	name[OBJECT_NAME_LENGTH];	/* name */
#else
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif /* CLANGSPEC */
#endif
} MPFCB;

IMPORT MPFCB knl_mpfcb_table[];	/* Fixed size memory pool control block */
IMPORT QUEUE knl_free_mpfcb;	/* FreeQue */

#define get_mpfcb(id)	( &knl_mpfcb_table[INDEX_MPF(id)] )


#define MINSIZE		( sizeof(FREEL) )
#define MINSZ(sz)	( ((size_t)(sz) + (MINSIZE-1)) & ~(MINSIZE-1) )

/*
 * Return end address in memory pool area
 */
Inline void *knl_mempool_end( MPFCB *mpfcb )
{
	return (VB*)mpfcb->mempool + mpfcb->mpfsz;
}

#define knl_mpf_chg_pri mpf_chg_pri

#endif /* _MEMPFIX_H_ */
