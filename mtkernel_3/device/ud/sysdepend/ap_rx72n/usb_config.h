/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usb_config.h
 */
#ifndef USB_CONFIG_H
#define USB_CONFIG_H

#define BSP_MCU_RX72N

/* USB control task priority. */
#define	USB_CFG_TASK_PRIORITY				(3)

/* USB interrupt priority level. */
#define	USB_CFG_INT_PRIORITY				(8)

/* USB vector number. */
#define	USB_CFG_VECTOR_NUMBER				(128)

/* USB DMA channel number. */
#define	USB_CFG_DMA_CHANNEL				(3)

/* USB power source. 0:Low Active, 1:High Active */
#define	USB_CFG_VBUS_ACTIVE				(1)

#endif	/* USB_CONFIG_H */