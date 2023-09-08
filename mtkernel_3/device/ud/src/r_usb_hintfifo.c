/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hintfifo.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"

/******************************************************************************
 Function Name   : usb_hstd_brdy_pipe
 Description     : BRDY Interrupt.
 Arguments       : uint16_t bitsts	: BRDYSTS Reg & BRDYENB Reg
 Return value    : none
 ******************************************************************************/
void usb_hstd_brdy_pipe(uint16_t bitsts)
{
	// When operating by the host function, usb_hstd_brdy_pipe() is executed without fail because
	// only one BRDY message is issued even when the demand of PIPE0 and PIPEx has been generated at the same time.
	if( USB_BRDY0 == ( bitsts & USB_BRDY0 ) )  {
		switch( g_usb_hstd_ctsq )  {			// Branch by the Control transfer stage management
		case USB_DATARD:		// Data stage of Control read transfer
                	switch( usb_hstd_read_data( ) )  {
			case USB_READEND:	// End of data read
			case USB_READSHRT:	// End of data read
				usb_hstd_status_start( );
				break;
			case USB_READING:	// Continue of data read
				break;
			case USB_READOVER:	// FIFO access error
				USB_PRINTF0("### Receive data over PIPE0 \n");
				usb_hstd_ctrl_end( USB_DATA_OVR );	// Control Read/Write End
				break;
			case USB_FIFOERROR:	// FIFO access error
				USB_PRINTF0("### FIFO access error \n");
				usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
				break;
			}
			break;
		case USB_DATARDCNT:		// Data stage of Control read transfer
			switch( usb_hstd_read_data( ) )  {
			case USB_READEND:	// End of data read
				usb_hstd_ctrl_end( USB_CTRL_READING );	// Control Read/Write End
				break;
			case USB_READSHRT:	// End of data read
				usb_hstd_status_start( );		// Control Read/Write Status
				break;
			case USB_READING:	// Continue of data read
				break;
			case USB_READOVER:	// FIFO access error
				USB_PRINTF0("### Receive data over PIPE0 \n");
				usb_hstd_ctrl_end( USB_DATA_OVR );	// Control Read/Write End
				break;
			case USB_FIFOERROR:	// FIFO access error
				USB_PRINTF0("### FIFO access error \n");
				usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
				break;
			}
			break;
		case USB_STATUSWR:		// Status stage of Control write (NoData control) transfer
			usb_hstd_ctrl_end( USB_CTRL_END );		// Control Read/Write End
			break;
		}
	}
	usb_hstd_brdy_pipe_process( bitsts );				// BRDY Interrupt
}

/******************************************************************************
 Function Name   : usb_hstd_nrdy_pipe
 Description     : NRDY Interrupt.
 Arguments       : uint16_t bitsts	: NRDYSTS Reg & NRDYENB Reg
 Return value    : none
 ******************************************************************************/
void usb_hstd_nrdy_pipe(uint16_t bitsts)
{
uint16_t buffer;

	// When operating by the host function, usb_hstd_nrdy_pipe() is executed without fail because
	// only one NRDY message is issued even when the demand of PIPE0 and PIPEx has been generated at the same time.
	if( USB_NRDY0 == ( bitsts & USB_NRDY0 ) )  {
		buffer = usb_cstd_get_pid( USB_PIPE0 );			// Get Pipe PID from pipe number
		if( USB_PID_STALL == (buffer & USB_PID_STALL) )  {	// STALL ?
			USB_PRINTF0("### STALL Pipe 0\n");
			usb_hstd_ctrl_end( USB_DATA_STALL );		// PIPE0 STALL callback
		}
		else  {
			g_usb_hstd_ignore_cnt[USB_PIPE0]++;		// Ignore count
			USB_PRINTF2("### IGNORE Pipe %d is %d times \n", USB_PIPE0, g_usb_hstd_ignore_cnt[USB_PIPE0]);
			if( USB_PIPEERROR == g_usb_hstd_ignore_cnt[USB_PIPE0] )	// Pipe error check
				usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Data Stage Device Ignore X 3 callback
			else  {						// Control Data Stage Retry
				usb_cpu_delay_xms( 5 );			// Wait 5ms
				USB0.DCPCTR.WORD &= ~USB_PID;		// PIPE0 Send IN or OUT token
				USB0.DCPCTR.WORD |= USB_PID_BUF;
			}
		}
	}
	usb_hstd_nrdy_pipe_process( bitsts );				// Nrdy Pipe interrupt
}

/******************************************************************************
 Function Name   : usb_hstd_bemp_pipe
 Description     : BEMP Interrupt.
 Arguments       : uint16_t bitsts  : BEMPSTS Reg & BEMPENB Reg
 Return value    : none
 ******************************************************************************/
void usb_hstd_bemp_pipe(uint16_t bitsts)
{
uint16_t buffer;
	// When operating by the host function, usb_hstd_bemp_pipe() is executed without fail because
	// only one BEMP message is issued even when the demand of PIPE0 and PIPEx has been generated at the same time.
	if( USB_BEMP0 == ( bitsts & USB_BEMP0 ) )  {
		buffer = usb_cstd_get_pid( USB_PIPE0 );			// Get Pipe PID from pipe number
		if( USB_PID_STALL == (buffer & USB_PID_STALL) )  {	// MAX packet size error ?
			USB_PRINTF0("### STALL Pipe 0\n");
			usb_hstd_ctrl_end( USB_DATA_STALL );		// PIPE0 STALL call back
		}
		else  {
			switch( g_usb_hstd_ctsq )  {	// Branch  by the Control transfer stage management
			case USB_DATAWR:	// Continuas of data stage (Control write)
				switch( usb_hstd_write_data( USB_PIPE0, USB_CUSE) )  {	// Buffer to CFIFO data write
				case USB_WRITESHRT:	// End of data write
					g_usb_hstd_ctsq = USB_STATUSWR;		// Next stage is Control write status stage
					USB0.BEMPENB.WORD |= 1 << USB_PIPE0;	// Enable Empty Interrupt
			        	USB0.NRDYENB.WORD |= 1 << USB_PIPE0;	// Enable Not Ready Interrupt
					break;
				case USB_WRITEEND:	// End of data write (not null)
				case USB_WRITING:	// Continue of data write
					USB0.BEMPENB.WORD |= 1 << USB_PIPE0;	// Enable Empty Interrupt
			        	USB0.NRDYENB.WORD |= 1 << USB_PIPE0;	// Enable Not Ready Interrupt
					break;
				case USB_FIFOERROR:	// FIFO access error
					USB_PRINTF0("### FIFO access error \n");
					usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
					break;
				}
				break;
			case USB_DATAWRCNT:	// Next stage to Control write data
				switch( usb_hstd_write_data( USB_PIPE0, USB_CUSE) )  {	// Buffer to CFIFO data write
				case USB_WRITESHRT:	// End of data write
					g_usb_hstd_ctsq = USB_STATUSWR;		// Next stage is Control write status stage
					USB0.BEMPENB.WORD |= 1 << USB_PIPE0;	// Enable Empty Interrupt
			        	USB0.NRDYENB.WORD |= 1 << USB_PIPE0;	// Enable Not Ready Interrupt
					break;
				case USB_WRITEEND:	// End of data write (not null)
					usb_hstd_ctrl_end( USB_CTRL_WRITING );	// Control Read/Write End
					break;
				case USB_WRITING:	// Continue of data write
					USB0.BEMPENB.WORD |= 1 << USB_PIPE0;	// Enable Empty Interrupt
			        	USB0.NRDYENB.WORD |= 1 << USB_PIPE0;	// Enable Not Ready Interrupt
					break;
				case USB_FIFOERROR:	// FIFO access error
					USB_PRINTF0("### FIFO access error \n");
					usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
					break;
				}
				break;
			case USB_STATUSWR:	// End of data stage (Control write)
				usb_hstd_status_start( );
				break;
			case USB_STATUSRD:	// Status stage of Control read transfer
				usb_hstd_ctrl_end( USB_CTRL_END );		// Control Read/Write End
				break;
			}
		}
	}
	usb_hstd_bemp_pipe_process( bitsts );			// BEMP interrupt
}