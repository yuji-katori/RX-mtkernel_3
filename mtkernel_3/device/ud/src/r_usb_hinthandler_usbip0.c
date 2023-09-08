/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hinthandler_usbip0.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"

static uint8_t   usb_cstd_int_msg_cnt;			// Interrupt message count
static usb_utr_t usb_cstd_int_msg[USB_INTMSGMAX];	// Interrupt message

/******************************************************************************
 Function Name   : usb_hstd_usb_handler
 Description     : USB interrupt routine. Analyze which USB interrupt occurred 
                   and send message to PCD/HCD task.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_usb_handler(void)
{
usb_utr_t *msg;
	msg = &usb_cstd_int_msg[ usb_cstd_int_msg_cnt ];	// Initial pointer
	usb_hstd_interrupt_handler( msg );			// Host Function Interrupt handler
	USB_SND_MSG( USB_HCD_MBX - 1, msg );			// Send message
	usb_cstd_int_msg_cnt++;					// Increment Message count
	if( USB_INTMSGMAX == usb_cstd_int_msg_cnt )		// Adjust Message count
		usb_cstd_int_msg_cnt = 0;
	USB_InterruptEvent( );					// Set USB Event(USB Interrupt)
}

/******************************************************************************
 Function Name   : usb_hstd_init_usb_message
 Description     : Initialization of message to be used at time of USB interrupt.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_init_usb_message(void)
{
	usb_cstd_int_msg[0].msginfo = USB_MSG_HCD_INT;
	usb_cstd_int_msg[1].msginfo = USB_MSG_HCD_INT;
}
