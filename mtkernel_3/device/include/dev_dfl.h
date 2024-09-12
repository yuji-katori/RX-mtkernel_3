/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	dev_dfl.h
 */
#ifndef __DEV_DFL_H__
#define __DEV_DFL_H__

#define	DFL_DEVNM	"DFLASH"

/* Data Flash Event Type */
typedef enum {
	TDV_BLANKCHECK4   = 0,
	TDV_BLANKCHECK64  = 1,
	TDV_BLOCKERASE64  = 2,
	TDV_BLOCKERASE128 = 3,
	TDV_BLOCKERASE256 = 4,
} DataFlashEventType;

IMPORT ER     DFL_Open(void);
IMPORT ER     DFL_Close(void);
IMPORT void   DFL_Read(T_DEVREQ *devreq);
IMPORT void   DFL_Write(T_DEVREQ *devreq);
IMPORT ER     DFL_Blank_Check(UW fsaddr, UW feaddr);
IMPORT ER     DFL_Block_Erase(UW fsaddr, UW feaddr);
IMPORT size_t DFL_GetFlashSize(void);
IMPORT ER     dflDrvEntry(void);

#endif /* __DEV_DFL_H__ */