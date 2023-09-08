/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hsignal.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"

/******************************************************************************
 Function Name   : usb_hstd_vbus_control
 Description     : USB VBUS ON/OFF setting.
 Arguments       : uint16_t command	: ON/OFF
 Return value    : none
 ******************************************************************************/
void usb_hstd_vbus_control(uint16_t command)
{
	if( USB_VBON == command )
		hw_usb_set_vbout( );
	else
		hw_usb_clear_vbout( );
}

/******************************************************************************
 Function Name   : usb_hstd_attach
 Description     : Set USB registers as required when USB device is attached, 
                   and notify MGR (manager) task that attach event occurred.
 Arguments       : uint16_t result	: Result
 Return value    : none
 ******************************************************************************/
void usb_hstd_attach(uint16_t result)
{
	usb_hstd_dtch_enable( );		// DTCH Interrupt Enable
	usb_hstd_berne_enable( );		// Interrupt Enable
	usb_hstd_notif_ator_detach( result );	// USB Mng API
}

/******************************************************************************
 Function Name   : usb_hstd_detach
 Description     : Set USB register as required when USB device is detached, and 
                   notify MGR (manager) task that detach occurred.
 Arguments       : uint16_t port	: Port number
 Return value    : none
 ******************************************************************************/
void usb_hstd_detach(void)
{							// DVSTCTR clear
	USB0.DVSTCTR0.WORD &= ~(USB_RWUPE | USB_USBRST | USB_RESUME | USB_UACT);
	usb_hstd_attch_enable( );			// ATTCH interrupt enable
	usb_hstd_notif_ator_detach( USB_DETACH );	// USB Mng API
}