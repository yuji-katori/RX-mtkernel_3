/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	dev_ud.h
 */
#ifndef __DEV_UD_H__
#define __DEV_UD_H__

#include <dev_disk.h>

// USB Memory Status
#define USB_NO_MEM		0
#define USB_RW_MEM		2

// USB Memory Kind
#define UNKNOWN_MEM		0
#define USB_MEM_STD		1

#define	BLOCK_SIZE		512

// EventFlag Bit Pattarn
#define USBEVENT		0x00000100
#define USBATTACH		0x00000200
#define USBDETACH		0x00000400
#define USBCAPACITY		0x00000800
#define EXECCMD			0x00000080
#define TSK_WAIT_ALL		0x00000F80
#define ATTACH			0x00001000
#define DETACH			0x00002000
#define MINIMUM			0x00002000
#define MAXIMUM			0x80000000

IMPORT usb_hmsc_strg_wait_cmdend( UB side );

IMPORT INT  USB_GetStatus(void);
IMPORT UINT USB_GetBlockCount(void);
IMPORT ER   USB_ReadBlock(void *buf, W start, SZ size);
IMPORT ER   USB_WriteBlock(void *buf, W start, SZ size);
IMPORT void USB_GetCapacity(void);

IMPORT UINT USB_Schedule(void);
IMPORT ER   USB_Init(ID flgid,T_DINT *p_dint);
IMPORT void USB_SetEvent(void);
IMPORT void USB_ClearEvent(BOOL flg);
IMPORT void USB_WaiEvent(void);
IMPORT PRI  USB_GetTaskPri(void);

#endif /* __DEV_UD_H__ */