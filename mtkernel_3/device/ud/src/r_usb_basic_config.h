/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_basic_config.h
 */

#ifndef R_USB_BASIC_CONFIG_H
#define R_USB_BASIC_CONFIG_H

/** [PLL clock frequency setting]
 * USB_CFG_24MHZ        : Set to 24MHz
 * USB_CFG_20MHZ        : Set to 20MHz
 * USB_CFG_OTHER        : Set to other than 24/20MHz
 */
#define USB_CFG_CLKSEL              (USB_CFG_24MHZ)

/* 
 * SET USB INTERRUPT PRIORITY; 
 *  1                   : lowest
 *  15                  : highest
 */
#define USB_CFG_INTERRUPT_PRIORITY  (USB_CFG_INT_PRIORITY)

/** [Setting power source IC for USB Host]
 * USB_CFG_HIGH         : High assert
 * USB_CFG_LOW          : Low assert
 */
#define USB_CFG_VBUS                (USB_CFG_VBUS_ACTIVE)

/** [Setting whther to use Type-C]
 * USB_CFG_ENABLE       : Using Type-C
 * USB_CFG_DISABLE      : Not using Type-C
 */
#define USB_CFG_TYPEC               (USB_CFG_DISABLE)

/** [DBLB bit setting]
 * USB_CFG_DBLBON       : DBLB bit set.
 * USB_CFG_DBLBOFF      : DBLB bit cleared.
 */
#define USB_CFG_DBLB                (USB_CFG_DBLBON)

#endif	/* R_USB_BASIC_CONFIG_H */
