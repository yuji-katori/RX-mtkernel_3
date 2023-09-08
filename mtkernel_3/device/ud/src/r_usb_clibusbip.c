/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_cdataio.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"

/******************************************************************************
 Function Name   : usb_cstd_get_pid
 Description     : Fetch specified pipe's PID.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : uint16_t PID-bit status
 ******************************************************************************/
uint16_t usb_cstd_get_pid(uint16_t pipe)
{
	return USB_PID & hw_usb_read_pipectr( pipe );	// PIPE control reg read */
}

/******************************************************************************
 Function Name   : usb_cstd_get_maxpacket_size
 Description     : Fetch MaxPacketSize of the specified pipe.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : uint16_t MaxPacketSize
 ******************************************************************************/
uint16_t usb_cstd_get_maxpacket_size(uint16_t pipe)
{
	if( USB_PIPE0 == pipe )
		return USB_MXPS & USB0.DCPMAXP.WORD;	// Max Packet Size
	else  {
		USB0.PIPESEL.WORD = USB_PIPE1;		// Pipe select
		return USB_MXPS & USB0.PIPEMAXP.WORD;	// Max Packet Size
	}
}

/******************************************************************************
 Function Name   : usb_cstd_get_pipe_type
 Description     : Fetch and return PIPE TYPE.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : uint16_t pipe type
 ******************************************************************************/
uint16_t usb_cstd_get_pipe_type(uint16_t pipe)
{
	USB0.PIPESEL.WORD = pipe;			// Pipe select
	return USB_TYPFIELD & USB0.PIPECFG.WORD;	// Read Pipe direction
}

/******************************************************************************
 Function Name   : usb_cstd_set_buf
 Description     : Set PID (packet ID) of the specified pipe to BUF.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : none
 ******************************************************************************/
void usb_cstd_set_buf(uint16_t pipe)
{
	if( USB_PIPE0 == pipe )  {
		USB0.DCPCTR.WORD &= ~USB_PID;
		USB0.DCPCTR.WORD |= USB_PID_BUF;
	}
	else  {
		USB0.PIPE1CTR.WORD &= ~USB_PID;
		USB0.PIPE1CTR.WORD |= USB_PID_BUF;
	}
}

/******************************************************************************
 Function Name   : usb_cstd_clr_stall
 Description     : Set up to NAK the specified pipe, and clear the STALL-bit set
                   to the PID of the specified pipe.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : none
 Note            : PID is set to NAK
 ******************************************************************************/
void usb_cstd_clr_stall(uint16_t pipe)
{
	usb_cstd_set_nak( pipe );			// Set NAK
	hw_usb_clear_pid( pipe, USB_PID_STALL );	// Clear STALL
}

/******************************************************************************
 Function Name   : usb_cstd_port_speed
 Description     : Get USB-speed of the specified port.
 Arguments       : none
 :Return value   : uint16_t	: HSCONNECT, Hi-Speed
                 :		: FSCONNECT : Full-Speed
                 :		: LSCONNECT : Low-Speed
                 :		: NOCONNECT : not connect
 ******************************************************************************/
uint16_t usb_cstd_port_speed(void)
{
	switch( USB_RHST & USB0.DVSTCTR0.WORD )  {		// Get port speed
	case USB_HSMODE :
		return USB_HSCONNECT;
	case USB_FSMODE :
		return USB_FSCONNECT;
	case USB_LSMODE :
		return USB_LSCONNECT;
	case USB_HSPROC :
		return USB_NOCONNECT;
	default :
		return USB_NOCONNECT;
	}
}

/******************************************************************************
 Function Name   : USB_Task
 Description     : USB driver main loop processing.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void USB_Task(void)
{
static void (* const task[])(usb_utr_t *) = {
USB_NULL, usb_hstd_hcd_task, usb_hstd_mgr_task, usb_hmsc_task, usb_hmsc_strg_drive_task };
#if defined(USB_DEBUG_ON)
static char * const task_name[] = { "", "HCD", "MGR", "HMSC", "STRG" };
#endif	/* defined(USB_DEBUG_ON) */
	while( usb_cstd_scheduler( ) == USB_OK )  {			// Receive Message
#if defined(USB_DEBUG_ON)
		USB_PRINTF3(" %-4s %04X %04X\n",task_name[g_usb_scheduler_id_use],
		g_usb_scheduler_dt_use->msginfo, g_usb_scheduler_dt_use->keyword);
#endif	/* defined(USB_DEBUG_ON) */
		task[g_usb_scheduler_id_use]( g_usb_scheduler_dt_use );
	}
}
