/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dev_iic.h
 */
#ifndef __DEV_IIC_H__
#define __DEV_IIC_H__

// EventFlag Bit Pattarn
#define IIC_EXECCMD		0x00000020
#define IIC_EEI_INT		0x00000018
#define IIC_NACK_INT		0x00000010
#define IIC_RXI_INT		0x00000004
#define IIC_TXI_INT		0x00000002
#define IIC_TEI_INT		0x00000001
#define IIC_MINIMUM		0x00000020
#define IIC_MAXIMUM		0x80000000

typedef struct st_iic_tbl {
	UB		rd;			// Read  Pointer
	UB		wt;			// Write Pointer
	ID		flgid;			// EventFlag ID
	UINT		now;			// Command Now  Position
	UINT		next;			// Command Next Position
	VB	       *drvname;		// IIC Driver Name
	void           *iic;			// IIC Channel Address
	W		lock;			// IIC Lock
	T_DEVREQ       *req[CFN_MAX_REQDEV+1];	// Device Reuest Pointer Array
} IIC_TBL;

IMPORT void drv_lock(INT mode, W *lock);
IMPORT void (*GroupBL1Table[])(UINT dintno);
IMPORT void GroupBL1Handler(UINT dintno);
IMPORT IIC_TBL iic_tbl[];
IMPORT void iic_tsk(INT stacd, void *exinf);
IMPORT ER   iicDriverEntry(VB *DrvName, UINT ch, T_DDEV *p_ddev);
IMPORT ER   IIC_Read(T_DEVREQ *devreq, void *exinf);
IMPORT ER   IIC_Write(T_DEVREQ *devreq, void *exinf);
IMPORT ER   iicDrvEntry(void);

#endif /* __DEV_IIC_H__ */