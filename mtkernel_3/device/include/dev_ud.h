/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2024/09/13.
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
#define NOTSUPPORT		0x00000800
#define DMACOMPLETE		0x00000040
#define EXECCMD			0x00000080
#define TSK_WAIT_ALL		0x00000FC0
#define STRGRWEEND		0x00000001
#define STRGRWEND		0x00000002
#define RW_WAIT_ALL		0x00000543
#define ATTACH			0x00001000
#define DETACH			0x00002000
#define MINIMUM			0x00002000
#define MAXIMUM			0x80000000

IMPORT void USB_Int_hdr(UINT intno);
IMPORT void DMA_End_hdr(UINT intno);
IMPORT UH   usb_hmsc_strg_read_sector(UB *buf, UW secno, UH seccnt, UW trans_byte);
IMPORT UH   usb_hmsc_strg_write_sector(UB *buf, UW secno, UH seccnt, UW trans_byte);
IMPORT void usb_hmsc_strg_read_capacity(void);
IMPORT void usb_cstd_dma_driver(void);
IMPORT void R_USB_Open(void);
IMPORT void R_USB_Close(void);

IMPORT void drv_lock(INT mode, W *lock);
IMPORT INT  USB_GetStatus(void);
IMPORT UINT USB_GetBlockCount(void);
IMPORT ER   USB_ReadBlock(void *buf, W start, SZ size);
IMPORT ER   USB_WriteBlock(void *buf, W start, SZ size);
IMPORT ER   USB_WaitRWEndEvent(void);

IMPORT void USB_Task(void);
IMPORT void USB_GetCapacity(UINT *BlockCount, UINT *BlockSize);
IMPORT void USB_Attach(void);
IMPORT void USB_Detach(void);
IMPORT void USB_NoSupport(void);
IMPORT ER   USB_Init(ID flgid,T_DINT *p_dint);
IMPORT PRI  USB_GetTaskPri(void);

#endif /* __DEV_UD_H__ */