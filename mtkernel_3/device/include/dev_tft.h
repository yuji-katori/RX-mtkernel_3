/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dev_tft.h
 */
#ifndef __DEV_TFT_H__
#define __DEV_TFT_H__

#define	TFT_DEVNM	"SCREEN"

IMPORT void drv_lock(INT mode, W *lock);
IMPORT SZ   TFT_Write(SZ size, CONST VB *str);
IMPORT void TFT_Init(void);
IMPORT ER   tftDrvEntry(void);

#endif /* __DEV_TFT_H__ */