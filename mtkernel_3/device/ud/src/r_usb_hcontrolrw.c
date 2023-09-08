/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hcontrolrw.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"

/******************************************************************************
 Function Name   : usb_hstd_ctrl_write_start
 Description     : Start data stage of Control Write transfer.
 Arguments       : uint32_t Bsize	: Data Size
                 : uint8_t  *Table	: Data Table Address
 Return          : uint16_t		: USB_WRITEEND / USB_WRITING /
                 :			: USB_WRITESHRT / USB_FIFOERROR
 ******************************************************************************/
uint16_t usb_hstd_ctrl_write_start(uint32_t Bsize, uint8_t *Table)
{
uint16_t end_flag;
	usb_cstd_clr_stall( USB_PIPE0 );			// PID=NAK & clear STALL
	g_usb_hstd_data_cnt[USB_PIPE0] = Bsize;			// Transfer size set
	g_usb_hstd_data_ptr[USB_PIPE0] = Table;			// Transfer address set
#if !defined(BSP_MCU_RX630)
	USB0.DCPCFG.WORD = USB_CNTMDFIELD | USB_DIRFIELD;	// DCP Configuration Register (0x5C)
#endif	/* !defined(BSP_MCU_RX630) */
	USB0.DCPCTR.WORD |= USB_SQSET;				// SQSET=1, PID=NAK
	if( USB_DATAWRCNT == g_usb_hstd_ctsq )  {		// Next stage is Control read data stage
		uint16_t toggle = g_usb_hstd_pipe[USB_PIPE0]->pipectr;
		usb_hstd_do_sqtgl( USB_PIPE0, toggle );		// Do pipe SQTGL
	}
	USB0.BEMPSTS.WORD = ~(1 << USB_PIPE0) & BEMPSTS_MASK;
	g_usb_hstd_ignore_cnt[USB_PIPE0] = 0;			// Ignore count clear
	end_flag = usb_hstd_write_data( USB_PIPE0, USB_CUSE);	// Host Control sequence
	switch( end_flag )  {
	case USB_WRITESHRT:		// End of data write
		g_usb_hstd_ctsq = USB_STATUSWR;			// Next stage is Control write status stage
		USB0.BEMPENB.WORD |= 1 << USB_PIPE0;		// Enable Empty Interrupt
	        USB0.NRDYENB.WORD |= 1 << USB_PIPE0;		// Enable Not Ready Interrupt
		USB0.DCPCTR.WORD &= ~USB_PID;			// Set BUF
		USB0.DCPCTR.WORD |=  USB_PID_BUF;
		break;
	case USB_WRITEEND:		// End of data write (not null)
        case USB_WRITING:		// Continue of data write
		if( USB_SETUPWR == g_usb_hstd_ctsq )
			g_usb_hstd_ctsq = USB_DATAWR;		// Next stage is Control write data stage
		else
			g_usb_hstd_ctsq = USB_DATAWRCNT;	// Next stage is Control read data stage
		USB0.BEMPENB.WORD |= 1 << USB_PIPE0;		// Enable Empty Interrupt
        	USB0.NRDYENB.WORD |= 1 << USB_PIPE0;		// Enable Not Ready Interrupt
		USB0.DCPCTR.WORD &= ~USB_PID;			// Set BUF
		USB0.DCPCTR.WORD |= USB_PID_BUF;
		break;
	case USB_FIFOERROR:		// FIFO access error
		break;
	}
	return end_flag;					// End or Err or Continue
}

/******************************************************************************
 Function Name   : usb_hstd_ctrl_read_start
 Description     : Start data stage of Control Read transfer.
 Arguments       : uint32_t Bsize	: Data Size
                 : uint8_t  *Table	: Data Table Address
 Return          : none
 ******************************************************************************/
void usb_hstd_ctrl_read_start(uint32_t Bsize, uint8_t *Table)
{
	usb_cstd_clr_stall( USB_PIPE0 );			// PID=NAK & clear STALL
	g_usb_hstd_data_cnt[USB_PIPE0] = Bsize;			// Transfer size set
	g_usb_hstd_data_ptr[USB_PIPE0] = Table;			// Transfer address set
#if !defined(BSP_MCU_RX630)
	USB0.DCPCFG.WORD = USB_SHTNAKFIELD;			// DCP Configuration Register (0x5C)
#endif	/* !defined(BSP_MCU_RX630) */
	USB0.DCPCTR.WORD = USB_SQSET;				// SQSET=1, PID=NAK
	if( USB_DATARDCNT == g_usb_hstd_ctsq )  {
		uint16_t toggle = g_usb_hstd_pipe[USB_PIPE0]->pipectr;
		usb_hstd_do_sqtgl( USB_PIPE0, toggle );		// Next stage is Control read data stage
	}
	if( USB_SETUPRD == g_usb_hstd_ctsq )			// Host Control sequence
		g_usb_hstd_ctsq = USB_DATARD;			// Next stage is Control read data stage
	else
		g_usb_hstd_ctsq = USB_DATARDCNT;		// Next stage is Control read data stage
	g_usb_hstd_ignore_cnt[USB_PIPE0] = 0;			// Ignore count clear
	USB0.BRDYENB.WORD |= 1 << USB_PIPE0;			// Enable Ready Interrupt
        USB0.NRDYENB.WORD |= 1 << USB_PIPE0;			// Enable Not Ready Interrupt
	USB0.DCPCTR.WORD &= ~USB_PID;				// Set BUF
	USB0.DCPCTR.WORD |=  USB_PID_BUF;
}

/******************************************************************************
 Function Name   : usb_hstd_status_start
 Description     : Start status stage of Control Command.
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_status_start(void)
{
uint16_t end_flag;
uint8_t buf[2];
	USB0.BEMPENB.WORD &= ~(1 << USB_PIPE0);		// BEMP0 Disable
	USB0.BRDYENB.WORD &= ~(1 << USB_PIPE0);		// BRDY0 Disable
	g_usb_hstd_pipe[USB_PIPE0]->tranlen = g_usb_hstd_data_cnt[USB_PIPE0];	// Transfer size set
	switch( g_usb_hstd_ctsq )  {			// Branch  by the Control transfer stage management
	case USB_DATARD:			// Control Read Data
	case USB_DATARDCNT:			// Control Read Data
		g_usb_hstd_ctsq = USB_DATARD;		// Control read Status
		end_flag = usb_hstd_ctrl_write_start( 0, buf );		// Control write start
		if( USB_FIFOERROR == end_flag )  {
			USB_PRINTF0("### FIFO access error \n");
			usb_hstd_ctrl_end( USB_DATA_ERR );		// Control Read/Write End
		}
		else
			g_usb_hstd_ctsq = USB_STATUSRD;			// Next stage is Control read status stage
		break;
	case USB_STATUSWR:			// Control Write Data
	case USB_SETUPNDC:			// NoData Control
		usb_hstd_ctrl_read_start( 0, buf );			// Control Read Status
		g_usb_hstd_ctsq = USB_STATUSWR;				// Next stage is Control write status stage
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_ctrl_end
 Description     : Call the user registered callback function that notifies 
                   completion of a control transfer.
 Arguments       : uint16_t status	: Transfer status
 Return          : none
 ******************************************************************************/
void usb_hstd_ctrl_end(uint16_t status)
{
	USB0.BEMPENB.WORD &= ~(1 << USB_PIPE0);				// BEMP0 Disable
	USB0.BRDYENB.WORD &= ~(1 << USB_PIPE0);				// BRDY0 Disable
	USB0.NRDYENB.WORD &= ~(1 << USB_PIPE0);				// NRDY0 Disable
	usb_cstd_clr_stall( USB_PIPE0 );				// PID=NAK & clear STALL
	hw_usb_set_mbw( USB_CUSE, USB0_CFIFO_MBW );
	USB0.DCPCTR.WORD = USB_SUREQCLR | USB_SQCLR;			// SUREQ=1, SQCLR=1, PID=NAK
	usb_cstd_chg_curpipe( USB_PIPE0, USB_CUSE, USB_FALSE );		// CFIFO buffer clear
	USB0.CFIFOCTR.WORD = USB_BCLR;					// Clear BVAL
	usb_cstd_chg_curpipe( USB_PIPE0, USB_CUSE, USB_ISEL );
	USB0.CFIFOCTR.WORD = USB_BCLR;					// Clear BVAL
	if( USB_CTRL_READING != status && USB_CTRL_WRITING != status )	// Host Control sequence
		g_usb_hstd_ctsq = USB_IDLEST;				// Next stage is idle
	g_usb_hstd_pipe[USB_PIPE0]->status  = status;
	g_usb_hstd_pipe[USB_PIPE0]->pipectr = hw_usb_read_pipectr( USB_PIPE0 );
	g_usb_hstd_pipe[USB_PIPE0]->errcnt  = g_usb_hstd_ignore_cnt[USB_PIPE0];
	if( USB_NULL != g_usb_hstd_pipe[USB_PIPE0] )			// Callback
		if( USB_NULL != g_usb_hstd_pipe[USB_PIPE0]->complete )	// Process Done Callback
			g_usb_hstd_pipe[USB_PIPE0]->complete( g_usb_hstd_pipe[USB_PIPE0], USB_NULL, USB_NULL );
	g_usb_hstd_pipe[USB_PIPE0] = USB_NULL;
}

/******************************************************************************
 Function Name   : usb_hstd_setup_start
 Description     : Start control transfer setup stage. (Set global function re-
                   quired to start control transfer, and USB register).
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_setup_start(void)
{
uint16_t *setup;
uint16_t segment, dir;
uint16_t setup_req, setup_val, setup_indx, setup_leng;
	segment = g_usb_hstd_pipe[USB_PIPE0]->segment;
	setup   = g_usb_hstd_pipe[USB_PIPE0]->setup;
	setup_req  = setup[0];		// Set Request data
	setup_val  = setup[1];		// Set Value data
	setup_indx = setup[2];		// Set Index data
	setup_leng = setup[3];		// Set Length data
	USB0.DCPMAXP.WORD = g_usb_hstd_dcp_register[setup[4]];	// Max Packet Size + Device Number select
	if( USB_TRAN_END == segment )				// Check Last flag
		if( g_usb_hstd_pipe[USB_PIPE0]->tranlen < setup_leng )
			setup_leng = g_usb_hstd_pipe[USB_PIPE0]->tranlen;
	if( setup_leng < g_usb_hstd_pipe[USB_PIPE0]->tranlen )
		g_usb_hstd_pipe[USB_PIPE0]->tranlen = setup_leng;
	dir = setup_req & USB_BMREQUESTTYPEDIR;			// Control sequence setting
	if( !setup_leng )					// Check wLength field
		if( !dir )					// Check Dir field
			if( USB_TRAN_END == segment )		// Check Last flag
				g_usb_hstd_ctsq = USB_SETUPNDC;	// Next stage is NoData control status stage
			else
				g_usb_hstd_ctsq = USB_IDLEST;	// Error
		else
			g_usb_hstd_ctsq = USB_IDLEST;		// Error
	else
		if( !dir )					// Check Dir field
			if( USB_TRAN_END == segment )		// Check Last flag
				g_usb_hstd_ctsq = USB_SETUPWR;	// Next stage is Control Write data stage
			else
				g_usb_hstd_ctsq = USB_SETUPWRCNT;// Next stage is Control Write data stage
		else
			if( USB_TRAN_END == segment )		// Check Last flag
				g_usb_hstd_ctsq = USB_SETUPRD;	// Next stage is Control read data stage
			else
				g_usb_hstd_ctsq = USB_SETUPRDCNT;// Next stage is Control read data stage
	if( USB_IDLEST == g_usb_hstd_ctsq )			// Control transfer idle stage ?
		usb_hstd_ctrl_end( USB_DATA_STOP );		// Control Read/Write End
	else  {							// SETUP request set
		USB0.USBREQ.WORD = setup_req;			// Set Request data
		USB0.USBVAL  = setup_val;			// Set Value data
		USB0.USBINDX = setup_indx;			// Set Index data
		USB0.USBLENG = setup_leng;			// Set Length data
		g_usb_hstd_ignore_cnt[USB_PIPE0] = 0;		// Ignore count clear
		USB0.INTSTS1.WORD = ~USB_SIGN & INTSTS1_MASK;
		USB0.INTSTS1.WORD = ~USB_SACK & INTSTS1_MASK;
		USB0.INTENB1.WORD |= USB_SIGNE;
		USB0.INTENB1.WORD |= USB_SACKE;
		USB0.DCPCTR.WORD  |= USB_SUREQ;
	}
}
