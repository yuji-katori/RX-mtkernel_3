/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hstdfunction.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"

/******************************************************************************
 Function Name   : usb_hdriver_init
 Description     : USB Host driver initialization.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hdriver_init(void)
{
	usb_cstd_sche_init( );			// Scheduler init
	usb_hstd_init_usb_message( );		// USB interrupt message initialize
	usb_hstd_mgr_open( );			// MGR open
	usb_hstd_hcd_open( );			// HCD open
	usb_hmsc_driver_start( );		// Init host class driver task
	usb_hstd_driver_registration(  );	// Host MSC class driver registration
}
