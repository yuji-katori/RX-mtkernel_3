/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dev_sci.h
 */
#ifndef __DEV_SCI_H__
#define __DEV_SCI_H__

// EventFlag Bit Pattarn
#define SCI_SNDCMD		0x00000001
#define SCI_RCVCMD		0x00000002
#define SCI_SNDEND		0x00000004
#define SCI_RCVEND		0x00000008
#define SCI_WAITALL		0x0000000F
#define SCI_MINIMUM		0x00000008
#define SCI_MAXIMUM		0x80000000

typedef enum {
	DN_RSMODE = -100			// Communication Mode
} RSDataNo;

typedef struct {
	UW	parity	:2 ;			// 0:Non,  1:Odd,    2:Even, 3:?
	UW	datalen	:2 ;			// 0:5bit, 1:6bit,   2:7bit, 3:8bit
	UW	stopbits:2 ;			// 0:1bit, 1:1.5bit, 2:2bit, 3:?
	UW		:2 ;			// reserve
	UW	baud	:24;			// Bit Rate
} RsMode;

typedef struct st_sci_tbl {
	ID		flgid;			// EventFlag ID
	UINT		now;			// Command Now Position
	UINT		next;			// Command Next Position
	void           *sci;			// SCI Channel Address
	VB	       *drvname;		// SCI Driver Name
	W		lock;			// SCI Lock
	RsMode		rsmode;			// Communication Mode
	UW		pclk;			// Peripheral Clock
	UB		srd;			// Send    Read  Pointer
	UB		swt;			// Send    Write Pointer
	UB		rrd;			// Receive Read  Pointer
	UB		rwt;			// Receive Write Pointer
	UH		rd;			// Receive Data Read  Pointer
	UH		wt;			// Receive Data Write Pointer
	T_DEVREQ       *swreq;			// Send    Wait Device Request
	T_DEVREQ       *rwreq;			// Receive Wait Device Request
	T_DEVREQ       *sreq[CFN_MAX_REQDEV+1];	// Send    Device Reuest Pointer Array
	T_DEVREQ       *rreq[CFN_MAX_REQDEV+1];	// Receive Device Reuest Pointer Array
} SCI_TBL;

IMPORT void drv_lock(INT mode, W *lock);
IMPORT void (*GroupAL0Table[])(UINT dintno);
IMPORT void GroupAL0Handler(UINT dintno);
IMPORT void (*GroupBL0Table[])(UINT dintno);
IMPORT void GroupBL0Handler(UINT dintno);
IMPORT void (*GroupBL1Table[])(UINT dintno);
IMPORT void GroupBL1Handler(UINT dintno);
IMPORT SCI_TBL sci_tbl[];
IMPORT void sci_tsk(INT stacd, void *exinf);
IMPORT ER   sciDriverEntry(VB *DrvName, UINT ch, T_DDEV *p_ddev);
IMPORT void SCI_Read(T_DEVREQ *devreq, void *exinf);
IMPORT void SCI_Write(T_DEVREQ *devreq, void *exinf);
IMPORT ER   sciDrvEntry(void);

#endif /* __DEV_SCI_H__ */