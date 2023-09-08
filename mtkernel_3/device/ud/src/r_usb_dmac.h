/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_dmac.h
 */

#ifndef R_USB_DMAC_H
#define R_USB_DMAC_H

#define	USB_FIFO_ACCESS_TYPE_32BIT	(0)		// FIFO port 32bit access
#define	USB_FIFO_ACCESS_TYPE_16BIT	(1)		// FIFO port 16bit access
#define	USB_FIFO_ACCESS_TYPE_8BIT	(2)		// FIFO port 8bit access

#define	USB_BIT_MBW32			(3)		// Mod 4(4Byte=32Bit)
#define	USB_BIT_MBW16			(1)		// Mod 2(2Byte=16Bit)

extern uint32_t g_usb_cstd_dma_size;			// DMA buffer size
extern uint16_t g_usb_cstd_dma_fifo;			// DMA FIFO buffer size

void     usb_cstd_dma_driver(void);
uint16_t usb_cstd_dma_get_crtb(void);
void     usb_cstd_dma_stop(void);
void     usb_cstd_dma_rcv_start(void);
void     usb_cstd_dma_snd_start(void);
void     DMA_End_hdr(void);

#endif	/* R_USB_DMAC_H */
