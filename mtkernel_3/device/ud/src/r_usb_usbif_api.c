/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_usbif_api.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_reg_access.h"

/******************************************************************************
 * Function Name   : R_USB_Open
 *****************************************************************************/
void R_USB_Open(void)
{
	R_DMACA_Open( );
	usb_module_start( );
	usb_hdriver_init( );			// USB driver initialization
	usb_cpu_usbint_init( );			// Setting MCU(USB interrupt init) register
	hw_usb_hmodule_init( );			// Setting USB relation register
#if USB_CFG_TYPEC == USB_CFG_DISABLE
	usb_hstd_vbus_control( USB_VBON );
	usb_cpu_delay_xms( 100 );		// Wait 100ms
#else	/*  USB_CFG_TYPEC == USB_CFG_DISABLE */
	usb_hstd_vbus_control( USB_VBOFF );
#endif	/*  USB_CFG_TYPEC == USB_CFG_DISABLE */
}

/******************************************************************************
 * Function Name   : R_USB_Close
 *****************************************************************************/
void R_USB_Close(void)
{
	R_DMACA_Close( );
	usb_module_stop( );
	usb_hstd_driver_release( );
	usb_hstd_clr_pipe_table_ip( );
}
