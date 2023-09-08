/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hlibusbip.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"
#include "r_usb_dmac.h"

/******************************************************************************
 Function Name   : usb_hstd_set_dev_addr
 Description     : Set USB speed (Full/Hi) of the connected USB Device.
 Arguments       : uint16_t devsel	: device select value
                 : uint16_t speed	: device speed
 Return value    : none
 ******************************************************************************/
void usb_hstd_set_dev_addr(uint16_t devsel, uint16_t speed)
{
	if( USB_DEVICE0 == devsel )  {	// USB_DEVICE0 >> USB_DEVADDRBIT
		USB0.DCPMAXP.WORD = USB_DEFPACKET;
		USB0.DEVADD0.WORD = speed;	// Set Device 0 Speed
	}
	else
		USB0.DEVADD1.WORD = speed;	// Set Device 1 Speed
}

/******************************************************************************
 Function Name   : usb_hstd_bchg_enable
 Description     : Enable BCHG interrupt for the specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_bchg_enable(void)
{
	USB0.INTSTS1.WORD  = ~USB_BCHG & INTSTS1_MASK;
	USB0.INTENB1.WORD |=  USB_BCHGE;
}

/******************************************************************************
 Function Name   : usb_hstd_bchg_disable
 Description     : Disable BCHG interrupt for specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_bchg_disable(void)
{
	USB0.INTSTS1.WORD  = ~USB_BCHG & INTSTS1_MASK;
	USB0.INTENB1.WORD &= ~USB_BCHGE;
}

/******************************************************************************
 Function Name   : usb_hstd_set_uact
 Description     : Start sending SOF to the connected USB device.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_set_uact(void)
{
	hw_usb_rmw_dvstctr( USB_UACT, USB_USBRST | USB_RESUME | USB_UACT );
}

/******************************************************************************
 Function Name   : usb_hstd_ovrcr_enable
 Description     : Enable OVRCR interrupt of the specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_ovrcr_enable(void)
{
	USB0.INTSTS1.WORD  = ~USB_OVRCR & INTSTS1_MASK;
	USB0.INTENB1.WORD |=  USB_OVRCRE;
}

/******************************************************************************
 Function Name   : usb_hstd_ovrcr_disable
 Description     : Disable OVRCR interrupt of the specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_ovrcr_disable(void)
{
	USB0.INTSTS1.WORD  = ~USB_OVRCR & INTSTS1_MASK;	// OVRCR Clear(INT_N edge sense)
	USB0.INTENB1.WORD &= ~USB_OVRCRE;		// Over-current disable
}

/******************************************************************************
 Function Name   : usb_hstd_attch_enable
 Description     : Enable ATTCH (attach) interrupt of the specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_attch_enable(void)
{
	USB0.INTSTS1.WORD  = ~USB_ATTCH & INTSTS1_MASK;	// ATTCH status Clear
	USB0.INTENB1.WORD |=  USB_ATTCHE;		// Attach enable
}

/******************************************************************************
 Function Name   : usb_hstd_attch_disable
 Description     : Disable ATTCH (attach) interrupt of the specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_attch_disable(void)
{
	USB0.INTSTS1.WORD  = ~USB_ATTCH & INTSTS1_MASK;	// ATTCH Clear(INT_N edge sense)
	USB0.INTENB1.WORD &= ~USB_ATTCHE;		// Attach disable
}

/******************************************************************************
 Function Name   : usb_hstd_dtch_enable
 Description     : Enable DTCH (detach) interrupt of the specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_dtch_enable(void)
{
	USB0.INTSTS1.WORD  = ~USB_DTCH & INTSTS1_MASK;	// DTCH Clear
	USB0.INTENB1.WORD |=  USB_DTCHE;		// Detach enable
}

/******************************************************************************
 Function Name   : usb_hstd_dtch_disable
 Description     : Disable DTCH (detach) interrupt of the specified USB port.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_dtch_disable(void)
{
	USB0.INTSTS1.WORD  = ~USB_DTCH & INTSTS1_MASK;	// DTCH Clear(INT_N edge sense)
	USB0.INTENB1.WORD &= ~USB_DTCHE;		// Detach disable
}

/******************************************************************************
 Function Name   : usb_hstd_chk_dev_addr
 Description     : Get device address configuration register from device
                   address.
 Arguments       : uint16_t devsel	: USB device address value
 Return value    : uint16_t		: USB speed
 ******************************************************************************/
uint16_t usb_hstd_chk_dev_addr(uint16_t devsel)
{
	if( USB_DEVICE0 == devsel )		// USB_DEVICE0 >> USB_DEVADDRBIT
		return USB0.DEVADD0.WORD;	// Get Device 0 Speed
	else
		return USB0.DEVADD1.WORD;	// Get Device 1 Speed
}
 
 /******************************************************************************
 Function Name   : usb_hstd_pipe_to_epadr
 Description     : Get the associated endpoint value of the specified pipe
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : uint8_t		: OK    : Endpoint nr + direction
                 :			: ERROR : Error
 ******************************************************************************/
uint8_t usb_hstd_pipe_to_epadr(uint16_t pipe)
{
uint16_t buffer;
	USB0.PIPESEL.WORD = pipe;				// Pipe select
	buffer = USB0.PIPECFG.WORD;				// Read Pipe direction
	return ( ( buffer & USB_DIRFIELD ^ USB_DIRFIELD ) << 3 ) + ( buffer & USB_EPNUMFIELD );
}

/******************************************************************************
 Function Name   : usb_hstd_berne_enable
 Description     : Enable BRDY/NRDY/BEMP interrupt.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_berne_enable(void)
{
	USB0.INTENB0.WORD |= USB_BEMPE | USB_NRDYE | USB_BRDYE;	// Enable BEMP, NRDY, BRDY
}

/******************************************************************************
 Function Name   : usb_hstd_do_sqtgl
 Description     : Toggle setting of the toggle-bit for the specified pipe by argument
 Arguments       : uint16_t pipe	: Pipe number
                 : uint16_t toggle	: Current toggle status.
 Return value    : none
 ******************************************************************************/
void usb_hstd_do_sqtgl(uint16_t pipe, uint16_t toggle)
{
	if( USB_SQMON == ( toggle & USB_SQMON ) )	// Check toggle
		hw_usb_set_sqset( pipe );		// Do pipe SQSET
	else
		hw_usb_set_sqclr( pipe );		// Do pipe SQCLR
}

/******************************************************************************
 Function Name   : usb_hstd_get_devsel
 Description     : Get device address from pipe number.
 Arguments       : none
 Return value    : uint16_t DEVSEL-bit status
 ******************************************************************************/
uint16_t usb_hstd_get_devsel(void)
{
	USB0.PIPESEL.WORD = USB_PIPE1;			// Pipe select
	return USB0.PIPEMAXP.WORD & USB_DEVSEL;
}

/******************************************************************************
 Function Name   : usb_hstd_get_device_address
 Description     : Get the device address associated with the specified pipe.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : uint16_t DEVSEL-bit status
 ******************************************************************************/
uint16_t usb_hstd_get_device_address(uint16_t pipe)
{
uint16_t buffer;
	if( USB_PIPE0 == pipe )  {
		buffer = USB0.DCPMAXP.WORD;
		return buffer & USB_DEVSEL;		// Device address
	}
	else  if( USB_TRUE == g_usb_pipe_table.use_flag )
		return g_usb_pipe_table.pipe_maxp & USB_DEVSEL;
	return USB_ERROR;
}
 
/******************************************************************************
 Function Name   : usb_hstd_write_data
 Description     : Switch PIPE, request the USB FIFO to write data, and manage 
                   the size of written data.
 Arguments       : uint16_t pipe	: Pipe number
                 : uint16_t pipemode	: CUSE/D0DMA
 Return value    : uint16_t end_flag
 ******************************************************************************/
uint16_t usb_hstd_write_data(uint16_t pipe, uint16_t pipemode)
{
uint16_t size;
uint16_t count;
uint16_t buffer;
uint16_t mxps;
uint16_t pid;
uint16_t end_flag;
	if( USB_PIPE0 == pipe )					// Changes FIFO port by the pipe
		buffer = usb_cstd_is_set_frdy( USB_PIPE0, USB_CUSE, USB_ISEL );
	else
		buffer = usb_cstd_is_set_frdy( USB_PIPE1, USB_D0USE, USB_FALSE) ;
	if( USB_FIFOERROR == buffer )				// Check error
		return USB_FIFOERROR;				// FIFO access error
	size = usb_cstd_get_buf_size( pipe );			// Data buffer size
	mxps = usb_cstd_get_maxpacket_size( pipe );		// Max Packet Size
	if( g_usb_hstd_data_cnt[pipe] <= size )  {		// Data size check
		count = g_usb_hstd_data_cnt[pipe];
		if( ! count )					// Data count check
			end_flag = USB_WRITESHRT;		// Null Packet is end of write
		else if( ! (count % mxps) )
			end_flag = USB_WRITESHRT;		// Short Packet is end of write
		else
			if( USB_PIPE0 == pipe )
				end_flag = USB_WRITING;		// Just Send Size
			else
				end_flag = USB_WRITEEND;	// Write continues
	}
	else  {
		end_flag = USB_WRITING;				// Write continues
		count = size;
	}
	pid = usb_cstd_get_pid( pipe );
	usb_cstd_set_nak( pipe );
	g_usb_hstd_data_ptr[pipe] = usb_hstd_write_fifo( count, pipemode, g_usb_hstd_data_ptr[pipe] );
	if( g_usb_hstd_data_cnt[pipe] < size )  {		// Check data count to remain
		g_usb_hstd_data_cnt[pipe] = 0;			// Clear data count
		buffer = hw_usb_read_fifoctr( pipemode );	// Read CFIFOCTR
		if( !( buffer & USB_BVAL ) )			// Check BVAL
			hw_usb_set_bval( pipemode );		// Short Packet
	}
	else
		g_usb_hstd_data_cnt[pipe] -= count;		// Total data count - count
	USB0.BEMPSTS.WORD = ~(1 << pipe) & BEMPSTS_MASK;
	if( USB_PID_BUF == pid )				// USB_PID_BUF ?
		usb_cstd_set_buf( pipe );
	return end_flag;
}

/******************************************************************************
 Function Name   : usb_hstd_receive_start
 Description     : Start data reception using CPU/DMA transfer to USB Host/USB
                   device.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_receive_start(void)
{
usb_utr_t *msg;
	msg = g_usb_hstd_pipe[USB_PIPE1];				// Evacuation pointer
	if( USB_TRAN_CONT == msg->segment )				// Check transfer count
		usb_hstd_do_sqtgl( USB_PIPE1, msg->pipectr );		// Sequence toggle
	usb_cstd_set_nak( USB_PIPE1 );					// Select NAK
	g_usb_hstd_data_cnt[USB_PIPE1] = msg->tranlen;			// Set data count
	g_usb_hstd_data_ptr[USB_PIPE1] = msg->tranadr;			// Set data pointer
	g_usb_hstd_ignore_cnt[USB_PIPE1] = 0;				// Ignore count clear
	g_usb_cstd_dma_fifo = usb_cstd_get_buf_size( USB_PIPE1 );	// Transfer FIFO Select
	g_usb_cstd_dma_size = g_usb_hstd_data_cnt[USB_PIPE1];		// Transfer data size
	usb_cstd_dma_rcv_start( );
}

/******************************************************************************
 Function Name   : usb_hstd_read_data
 Description     : Request to read data from USB FIFO, and manage the size of 
                   the data read.
 Arguments       : none
 Return value    : USB_READING / USB_READEND / USB_READSHRT / USB_READOVER
 ******************************************************************************/
uint16_t usb_hstd_read_data(void)
{
uint16_t count;
uint16_t buffer;
uint16_t mxps;
uint16_t dtln;
uint16_t end_flag;
	buffer = usb_cstd_is_set_frdy( USB_PIPE0, USB_CUSE, USB_FALSE );	// Changes FIFO port by the pipe
	if( USB_FIFOERROR == buffer )
		return (USB_FIFOERROR);			// FIFO access error
	dtln = buffer & USB_DTLN;
	mxps = USB_MXPS & USB0.DCPMAXP.WORD;		// Max Packet Size
	if( g_usb_hstd_data_cnt[USB_PIPE0] < dtln )  {	// Buffer Over ?
		end_flag = USB_READOVER;
		usb_cstd_set_nak( USB_PIPE0 );		// Set NAK
		count = g_usb_hstd_data_cnt[USB_PIPE0];
		g_usb_hstd_data_cnt[USB_PIPE0] = dtln;
	}
	else if( g_usb_hstd_data_cnt[USB_PIPE0] == dtln )  {	// Just Receive Size
		count = dtln;
		end_flag = USB_READEND;
		usb_cstd_set_nak( USB_PIPE0 );		// Set NAK
	}
	else  {						// Continus Receive data
		count = dtln;
		end_flag = USB_READING;
		if( ! count || ( count % mxps ) )  {	// Null Packet receive
			end_flag = USB_READSHRT;
			usb_cstd_set_nak( USB_PIPE0 );	// Select NAK
		}
	}
	if( ! dtln )					// 0 length packet
		hw_usb_set_bclr( USB_CUSE );		// Clear BVAL
	else
		g_usb_hstd_data_ptr[USB_PIPE0] = usb_hstd_read_fifo( count, g_usb_hstd_data_ptr[USB_PIPE0] );
	g_usb_hstd_data_cnt[USB_PIPE0] -= count;
	return end_flag;
}

/******************************************************************************
 Function Name   : usb_hstd_data_end
 Description     : Set USB registers as appropriate after data transmission/re-
                   ception, and call the callback function as transmission/recep-
                   tion is complete.
 Arguments       : uint16_t status	: Transfer status type
 Return value    : none
 ******************************************************************************/
void usb_hstd_data_end(uint16_t status)
{
	usb_cstd_set_nak( USB_PIPE1 );			// Set NAK
	USB0.BRDYENB.WORD &= ~(1 << USB_PIPE1);		// Disable Ready Interrupt
	USB0.NRDYENB.WORD &= ~(1 << USB_PIPE1);		// Disable Not Ready Interrupt
	USB0.BEMPENB.WORD &= ~(1 << USB_PIPE1);		// Disable Empty Interrupt
	usb_cstd_clr_transaction_counter( USB_PIPE1 );	// Disable Transaction count
	USB0.D0FIFOSEL.WORD &= ~USB_DCLRM;	// DMA buffer clear mode clear
	hw_usb_set_mbw( USB_D0USE, USB0_D0FIFO_MBW );
	if( USB_NULL != g_usb_hstd_pipe[USB_PIPE1] )  {	// Transfer information set
		g_usb_hstd_pipe[USB_PIPE1]->tranlen = g_usb_hstd_data_cnt[USB_PIPE1];
		g_usb_hstd_pipe[USB_PIPE1]->status  = status;
		g_usb_hstd_pipe[USB_PIPE1]->pipectr = hw_usb_read_pipectr( USB_PIPE1 );
		g_usb_hstd_pipe[USB_PIPE1]->errcnt  = g_usb_hstd_ignore_cnt[USB_PIPE1];
		g_usb_hstd_pipe[USB_PIPE1]->complete( g_usb_hstd_pipe[USB_PIPE1], USB_NULL, USB_NULL );
		g_usb_hstd_pipe[USB_PIPE1] = USB_NULL;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_brdy_pipe_process
 Description     : Search for the PIPE No. that BRDY interrupt occurred, and 
                   request data transmission/reception from the PIPE.
 Arguments       : uint16_t bitsts	: BRDYSTS Register & BRDYENB Register
 Return value    : none
 ******************************************************************************/
void usb_hstd_brdy_pipe_process(uint16_t bitsts)
{
uint16_t buffer;
uint16_t maxps;
uint16_t set_dma_block_cnt;
uint16_t trans_dma_block_cnt;
uint16_t status;
	if( bitsts & USB_BITSET( USB_PIPE1) )  {		// Interrupt check
		USB0.BEMPSTS.WORD = ~(1 << USB_PIPE1) & BEMPSTS_MASK;
		if( USB_NULL != g_usb_hstd_pipe[USB_PIPE1] )  {
			maxps = g_usb_cstd_dma_fifo;
			USB0.D0FIFOSEL.WORD &= ~USB_DREQE;	// DMA Transfer request disable
			usb_cstd_dma_stop( );			// DMA stop
			buffer = usb_cstd_is_set_frdy( USB_PIPE1, USB_D0USE, USB_FALSE );
								// Changes FIFO port by the pipe
			set_dma_block_cnt = ( g_usb_hstd_data_cnt[USB_PIPE1] - 1 ) / g_usb_cstd_dma_fifo + 1;
			trans_dma_block_cnt = usb_cstd_dma_get_crtb( );
			g_usb_cstd_dma_size = buffer & USB_DTLN;	// Get D0fifo Receive Data Length
			if( set_dma_block_cnt > trans_dma_block_cnt )  {
				if( ! g_usb_cstd_dma_size )		// DTLN = 0 (Received 0 length packet)
					g_usb_cstd_dma_size += ((set_dma_block_cnt - trans_dma_block_cnt) * maxps);
				else
					g_usb_cstd_dma_size += ((set_dma_block_cnt - (trans_dma_block_cnt + 1)) * maxps);
			}
			if( g_usb_cstd_dma_size == g_usb_hstd_data_cnt[USB_PIPE1] )	// Check data count
				status = USB_DATA_OK;
			else if( g_usb_cstd_dma_size > g_usb_hstd_data_cnt[USB_PIPE1] )
				status = USB_DATA_OVR;
			else
				status = USB_DATA_SHT;
			g_usb_hstd_data_cnt[USB_PIPE1] -= g_usb_cstd_dma_size;	// received data size
			usb_hstd_data_end( status );		// End of data transfer
			USB0.D0FIFOCTR.WORD = USB_BCLR;		// Set BCLR
		}
	}
}

/******************************************************************************
 Function Name   : usb_hstd_nrdy_pipe_process
 Description     : Search for PIPE No. that occurred NRDY interrupt, and execute 
                   the process for PIPE when NRDY interrupt occurred.
 Arguments       : uint16_t     bitsts  : NRDYSTS Register & NRDYENB Register
 Return value    : none
 ******************************************************************************/
void usb_hstd_nrdy_pipe_process(uint16_t bitsts)
{
	if( (bitsts & USB_BITSET( USB_PIPE1 )) && USB_NULL != g_usb_hstd_pipe[USB_PIPE1] )	// Interrupt check
		if( USB_TYPFIELD_ISO == usb_cstd_get_pipe_type( USB_PIPE1 ) )
			if( USB_OVRN == ( USB0.FRMNUM.WORD & USB_OVRN ) )  {
				usb_hstd_forced_termination( USB_DATA_OVR );	// End of data transfer
				USB_PRINTF1("###ISO OVRN %d\n", g_usb_hstd_data_cnt[USB_PIPE1]);
			}
			else
				usb_hstd_forced_termination( USB_DATA_ERR );	// End of data transfer
		else
			usb_hstd_nrdy_endprocess( );
}

/******************************************************************************
 Function Name   : usb_hstd_bemp_pipe_process
 Description     : Search for PIPE No. that BEMP interrupt occurred, and
                   complete data transmission for the PIPE.
 Arguments       : uint16_t bitsts	: BEMPSTS Register & BEMPENB Register
 Return value    : none
 ******************************************************************************/
void usb_hstd_bemp_pipe_process(uint16_t bitsts)
{
	if( (bitsts & USB_BITSET( USB_PIPE1 )) && USB_NULL != g_usb_hstd_pipe[USB_PIPE1] )
		if( usb_cstd_get_pid( USB_PIPE1 ) & USB_PID_STALL )  {
			USB_PRINTF0("### STALL Pipe");		// MAX packet size error ?
			usb_hstd_forced_termination( USB_DATA_STALL );
		}
		else if( USB_INBUFM != ( hw_usb_read_pipectr( USB_PIPE1 ) & USB_INBUFM ) )  {
			USB0.BEMPSTS.WORD = ~(1 << USB_PIPE1) & BEMPSTS_MASK;
			usb_hstd_data_end( USB_DATA_NONE );			// End of data transfer
		}
}

/******************************************************************************
Function Name   : usb_hstd_make_pipe_reg_info
Description     : Make value for USB PIPE registers set value.
Arguments       : uint16_t devadr	: USB Device address
                : uint8_t  *descriptor	: Address for End Point Descriptor
                : usb_pipe_table_reg_t *pipe_table_work : Address for Store PIPE reg set value.
Return value    : Pipe no (USB_PIPE1:OK, USB_NULL:Error)
******************************************************************************/
uint8_t usb_hstd_make_pipe_reg_info(uint16_t devadr, uint8_t *descriptor, usb_pipe_table_reg_t *pipe_table_work)
{
uint16_t pipe_cfg;
uint16_t pipe_maxp;
	if( USB_DT_ENDPOINT != descriptor[USB_DEV_B_DESCRIPTOR_TYPE] )	// Check Endpoint descriptor
		return USB_NULL;
	switch( descriptor[USB_EP_B_ATTRIBUTES] & USB_EP_TRNSMASK )  {	// set pipe configuration value
	case USB_EP_BULK:		// Bulk Endpoint
		if( USB_EP_IN == ( descriptor[USB_EP_B_ENDPOINTADDRESS] & USB_EP_DIRMASK ) )	// IN(rcv)
			pipe_cfg = USB_TYPFIELD_BULK | USB_CFG_DBLB | USB_SHTNAKFIELD | USB_DIR_H_IN;
		else										// OUT(snd)
			pipe_cfg = USB_TYPFIELD_BULK | USB_CFG_DBLB | USB_DIR_H_OUT;
		break;
	default:
		return USB_NULL;
	}
	pipe_cfg  |= descriptor[USB_EP_B_ENDPOINTADDRESS] & USB_EP_NUMMASK;	// Endpoint number set
	pipe_maxp  = descriptor[USB_EP_B_MAXPACKETSIZE_L] | ( devadr << USB_DEVADDRBIT );
	pipe_maxp |= descriptor[USB_EP_B_MAXPACKETSIZE_H] << 8;			// set max packet size
	pipe_table_work->pipe_cfg  = pipe_cfg;					// Store PIPE reg set value
	pipe_table_work->pipe_maxp = pipe_maxp;
	return USB_PIPE1;
}

/******************************************************************************
Function Name   : usb_hstd_clr_pipe_table
Description     : Clear pipe table.
Arguments       : uint16_t devadr	: USB Device address
Return value    : none
******************************************************************************/
void usb_hstd_clr_pipe_table(uint16_t devadr)
{
	if( USB_TRUE == g_usb_pipe_table.use_flag )	// Check USB Device address
		if( (devadr << USB_DEVADDRBIT) == (g_usb_pipe_table.pipe_maxp & USB_DEVSEL) )  {
			g_usb_pipe_table.use_flag  = USB_FALSE;
			g_usb_pipe_table.pipe_cfg  = USB_NULL;
			g_usb_pipe_table.pipe_maxp = USB_NULL;
		}
}

/******************************************************************************
Function Name   : usb_hstd_clr_pipe_table_ip
Description     : Clear pipe table.
Arguments       : none
Return value    : none
******************************************************************************/
void usb_hstd_clr_pipe_table_ip(void)
{
	if( USB_TRUE == g_usb_pipe_table.use_flag )  {	// Check USB Device address
		g_usb_pipe_table.use_flag  = USB_FALSE;
		g_usb_pipe_table.pipe_cfg  = USB_NULL;
		g_usb_pipe_table.pipe_maxp = USB_NULL;
	}
}

/******************************************************************************
Function Name   : usb_hstd_set_pipe_reg
Description     : Set up USB registers to use specified pipe (Pipe unit).
Arguments       : none
Return value    : none
******************************************************************************/
void usb_hstd_set_pipe_reg(void)
{
	if( USB_TRUE == g_usb_pipe_table.use_flag )  {		// Check use block
		if( ( USB0.D0FIFOSEL.WORD & USB_CURPIPE ) == USB_PIPE1 )
			usb_cstd_chg_curpipe( USB_PIPE0, USB_D0USE, USB_FALSE );
		usb_cstd_pipe_init( );				// PIPE1 Setting
	}
}
