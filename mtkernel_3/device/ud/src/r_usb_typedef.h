/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_typedef.h
 */

#ifndef R_USB_TYPEDEF_H
#define R_USB_TYPEDEF_H

#include <stdint.h>
#include "r_usb_basic_if.h"

typedef	uint16_t usb_er_t;
typedef struct usb_utr	usb_utr_t;
typedef void (*usb_cb_t)(struct usb_utr *, uint16_t, uint16_t);

typedef struct usb_utr {
	usb_utr_t      *msghead;	// Message Header
	uint8_t		errcnt;		// Error count
	uint8_t		segment;	// Last flag
	uint16_t	msginfo;	// Message Info for F/W
	uint16_t	keyword;	// Root port/Device address/Pipe number
	uint16_t	result;		// Result
	uint16_t	status;		// Status
	uint16_t	pipectr;	// Pipe control register
	uint32_t	tranlen;	// Transfer data length
	uint16_t       *setup;		// Setup packet(for control only)
	void	       *tranadr;	// Transfer data Start address
	usb_cb_t	complete;	// Call Back Function Info
} usb_message_t;

typedef struct usb_ctrl_trans {
	usb_setup_t	setup;		// Request command
	uint16_t	address;	// Device address setting
} usb_ctrl_trans_t;

typedef struct usb_pipe_table {
	uint16_t	use_flag;
	uint16_t	pipe_cfg;
	uint16_t	pipe_maxp;
} usb_pipe_table_t;

typedef struct usb_pipe_reg {
	uint16_t	pipe_cfg;
	uint16_t	pipe_maxp;
} usb_pipe_table_reg_t;

typedef union usb_descriptor {
	uint8_t		byte;
	uint16_t	word;
} usb_descriptor_t;

#endif	/* R_USB_TYPEDEF_H */
