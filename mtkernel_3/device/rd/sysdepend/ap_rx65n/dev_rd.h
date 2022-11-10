/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	dev_rd.h
 */

#define	TOP_ADDRESS	0x8000000
#define	END_ADDRESS	0x9000000
#define	BLOCK_SIZE	512
#define	BLOCK_COUNT	(( END_ADDRESS - TOP_ADDRESS ) / 512)

#define RAM_DISK_DEVNM	"rda"

IMPORT	ER	rdDrvEntry( void );
