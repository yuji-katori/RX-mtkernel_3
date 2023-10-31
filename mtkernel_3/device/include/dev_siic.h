/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dev_siic.h
 */
#ifndef __DEV_SIIC_H__
#define __DEV_SIIC_H__

// EventFlag Bit Pattarn
#define TSK_WAIT_ALL		0x00000004
#define EXECCMD			0x00000004
#define RXI_INT			0x00000001
#define TXI_INT			0x00000002
#define MINIMUM			0x00000008
#define MAXIMUM			0x80000000

typedef struct st_siic_tbl {
	UB		rd;			// Read  Pointer
	UB		wt;			// Write Pointer
	ID		flgid;			// EventFlag ID
	UINT		now;			// Command Now Position
	UINT		next;			// Command Next Position
	void           *siic;			// SIIC Channel Address
	W		lock;			// SIIC Lock
	T_DEVREQ       *req[CFN_MAX_REQDEV+1];	// Device Reuest Pointer Array
} SIIC_TBL;

IMPORT SIIC_TBL siic_tbl[];
IMPORT void siic_tsk(INT stacd, void *exinf);
IMPORT ER   siicDriverEntry(VB *DrvName, UINT ch, T_DDEV *p_ddev);
IMPORT ER   SIIC_Read(T_DEVREQ *devreq, void *exinf);
IMPORT ER   SIIC_Write(T_DEVREQ *devreq, void *exinf);
IMPORT ER   siicDrvEntry(void);

#endif /* __DEV_SIIC_H__ */