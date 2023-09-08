/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hdriver.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"
#include "r_usb_dmac.h"

static uint8_t  usb_hstd_pipe_request[USB_MAX_PIPE];
static uint8_t  usb_hstd_clr_stall_pipe;
static uint16_t usb_hstd_clr_stall_request[5];  
static usb_cb_t usb_hstd_clr_stall_call;
static void     usb_hstd_set_submitutr(usb_utr_t *msg);
static void     usb_hstd_clr_stall_result(usb_utr_t *msg, uint16_t data1, uint16_t data2);

uint8_t    g_usb_hstd_ctsq;				// Control transfer stage management
uint8_t    g_usb_hstd_mgr_mode;				// Manager mode
uint8_t    g_usb_hstd_device_state;			// Device state
uint16_t   g_usb_hstd_device_speed;			// Reset handshake result
uint16_t   g_usb_hstd_device_addr;			// Device address
uint8_t    g_usb_hstd_ignore_cnt[USB_MAX_PIPE];		// Ignore count
uint16_t   g_usb_hstd_device_info[5];			// Root port, status, config num, interface class, speed
uint16_t   g_usb_hstd_dcp_register[USB_MAXDEVADR];	// DEVSEL & DCPMAXP (Multiple device)
uint32_t   g_usb_hstd_data_cnt[USB_MAX_PIPE];		// PIPEn Buffer counter
uint8_t   *g_usb_hstd_data_ptr[USB_MAX_PIPE];		// PIPEn Buffer pointer
usb_utr_t *g_usb_hstd_pipe[USB_MAX_PIPE];		// Message pipe

/******************************************************************************
 Function Name   : usb_hstd_transfer_start_req
 Description     : Send a request for data transfer to HCD (Host Control Driver) 
                   using the specified pipe.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
 Return          : usb_er_t		: USB_OK/USB_QOVR/USB_ERROR
 ******************************************************************************/
usb_er_t usb_hstd_transfer_start_req(usb_utr_t *msg)
{
uint16_t pipe;
uint16_t devsel;
uint16_t retval;
	pipe = msg->keyword;
	if( USB_PIPE0 == pipe )
		devsel = msg->setup[4] << USB_DEVADDRBIT;
	else
		devsel = usb_hstd_get_devsel( );		// Get device address from pipe number
	if( USB_DEVICE0 == devsel && USB_PIPE0 != pipe )  {
		USB_PRINTF1("### usb_hstd_transfer_start_req not configured %x\n", devsel);
		retval = USB_ERROR;
		goto Error;
	}
	msg->msginfo = USB_MSG_HCD_SUBMITUTR;
	if( USB_ON == usb_hstd_pipe_request[pipe] )  {
		retval = USB_QOVR;
		goto Error;
	}
	if( USB_NULL != g_usb_hstd_pipe[pipe] )			// Pipe Transfer Process check
		if( usb_cstd_get_pipe_type( pipe ) != USB_TYPFIELD_ISO )  {	// Check PIPE TYPE
			USB_PRINTF1("### usb_hstd_transfer_start_req overlaps %d\n", pipe);
			retval = USB_QOVR;
			goto Error;
		}
	if( USB_NOCONNECT == usb_hstd_chk_dev_addr( devsel) )  {// Get device speed from device address
		USB_PRINTF1("### usb_hstd_transfer_start_req not connect %x\n", devsel);
		retval = USB_ERROR;
		goto Error;
	}
	USB_SND_MSG( USB_HCD_MBX, msg );			// Send Message
        usb_hstd_pipe_request[pipe] = USB_ON;
	return USB_OK;
Error:
	if( g_usb_scheduler_block <= msg  && msg <= &g_usb_scheduler_block[USB_BLKMAX-1] )
	USB_REL_BLK( msg );					// Release Message Block
	return retval;
}

/******************************************************************************
 Function Name   : usb_hstd_hcd_snd_mbx
 Description     : Send specified message to HCD (Host Control Driver) task.
 Arguments       : uint16_t msginfo	: Message information
                 : uint16_t pipe	: Pipe number
                 : uint16_t *adr	: Address
                 : usb_cb_t callback	: Callback function pointer
 Return          : none
 ******************************************************************************/
void usb_hstd_hcd_snd_mbx(uint16_t msginfo, uint16_t pipe, uint16_t *adr, usb_cb_t callback)
{
usb_utr_t *msg;
	msg = USB_GET_BLK( );			// Get Message Block
	msg->msginfo = msginfo;
	msg->keyword = pipe;
	msg->tranadr = adr;
	msg->complete = callback;
	USB_SND_MSG( USB_HCD_MBX, msg );	// Send Message
}

/******************************************************************************
 Function Name   : usb_hstd_mgr_snd_mbx
 Description     : Send the message to MGR(Manager) task.
 Arguments       : uint16_t msginfo	: Message information
                 : uint16_t pipe	: Port number
                 : uint16_t res		: Result
 Return          : none
 ******************************************************************************/
void usb_hstd_mgr_snd_mbx (uint16_t msginfo, uint16_t pipe, uint16_t result)
{
usb_utr_t *msg;
	msg = USB_GET_BLK( );			// Get Message Block
        msg->msginfo = msginfo;
        msg->keyword = pipe;
        msg->result = result;
	USB_SND_MSG( USB_MGR_MBX, msg );	// Send Message
}

/******************************************************************************
 Function Name   : usb_hstd_set_submitutr
 Description     : Submit utr: Get the device address via the specified pipe num-
                   ber and do a USB transfer.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
 Return          : none
 ******************************************************************************/
static void usb_hstd_set_submitutr(usb_utr_t *msg)
{
uint16_t pipe;
uint16_t devsel;
	pipe = msg->keyword;
	g_usb_hstd_pipe[pipe] = msg;
	usb_hstd_pipe_request[pipe] = USB_OFF;
	if( USB_PIPE0 == pipe )					// Get device address from pipe number
		devsel = msg->setup[4] << USB_DEVADDRBIT;
	else
		devsel = usb_hstd_get_devsel( );		// Get device address from pipe number

	if( USB_NOCONNECT == usb_hstd_chk_dev_addr( devsel) )  {// Get device speed from device address
		if( USB_PIPE0 == pipe )
			usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
		else
			usb_hstd_forced_termination( USB_DATA_ERR );
		return;						// End of data transfer (IN/OUT)
	}
	if( USB_DEVICE0 == devsel && USB_PIPE0 != pipe )  {
		usb_hstd_forced_termination( USB_DATA_ERR );
		return;
	}
	if( USB_PIPE0 == pipe )					// Control Transfer
		if( USB_IDLEST == g_usb_hstd_ctsq )		// Control transfer idle stage ?
			usb_hstd_setup_start( );
		else if( USB_DATARDCNT == g_usb_hstd_ctsq )  {	// Control Read Data
			msg = g_usb_hstd_pipe[USB_PIPE0];
			usb_hstd_ctrl_read_start( msg->tranlen, msg->tranadr);
		} 						// Control read start
		else if( USB_DATAWRCNT == g_usb_hstd_ctsq )  {	// Control Write Data
			msg = g_usb_hstd_pipe[USB_PIPE0];	// Control write start
			if( USB_FIFOERROR == usb_hstd_ctrl_write_start( msg->tranlen, msg->tranadr) )  {
				USB_PRINTF0("### FIFO access error\n");
				usb_hstd_ctrl_end( USB_DATA_ERR );
			}					// Control Read/Write End
		}
		else  {
			USB_PRINTF0("### Control transfer sequence error\n");
			usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
		}
	else  {
		USB0.PIPESEL.WORD = USB_PIPE1;			// Pipe select
		if( ( USB_DIRFIELD & USB0.PIPECFG.WORD ) == USB_DIR_H_IN )	// Data Transfer
			usb_hstd_receive_start( );		// IN Transfer
		else
			usb_hstd_send_start( );			// OUT Transfer
	}
}

/******************************************************************************
 Function Name   : usb_hstd_bus_int_disable
 Description     : Disable USB Bus Interrupts OVRCR, ATTCH, DTCH, and BCHG.
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_bus_int_disable(void)
{
	usb_hstd_attch_disable( );		// ATTCH interrupt disable
	usb_hstd_dtch_disable( );		// DTCH  interrupt disable
	usb_hstd_bchg_disable( );		// BCHG  interrupt disable
}

/******************************************************************************
 Function Name   : usb_hstd_interrupt
 Description     : Execute appropriate process depending on which USB interrupt 
                   occurred.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
 Return          : none
 ******************************************************************************/
static void usb_hstd_interrupt(usb_utr_t *msg)
{
uint16_t intsts = msg->keyword;
uint16_t bitsts = msg->status;
	switch( intsts )  {
	/***** Processing PIPE0-MAX_PIPE_NO data *****/
	case USB_INT_BRDY:
		usb_hstd_brdy_pipe( bitsts );
		break;
	case USB_INT_BEMP :
		usb_hstd_bemp_pipe( bitsts );
		break;
	case USB_INT_NRDY :
		usb_hstd_nrdy_pipe( bitsts );
		break;
	/***** Processing Setup transaction *****/
        case USB_INT_SACK :
		switch( g_usb_hstd_ctsq )  {
		case USB_SETUPRD:	// Next stage to Control read data
		case USB_SETUPRDCNT:
			msg = g_usb_hstd_pipe[USB_PIPE0];
			usb_hstd_ctrl_read_start( msg->tranlen, msg->tranadr);
			break;						// Control read start
		case USB_SETUPWR:	// Next stage to Control Write data
                case USB_SETUPWRCNT:
			msg = g_usb_hstd_pipe[USB_PIPE0];		// Control write start
			if( USB_FIFOERROR == usb_hstd_ctrl_write_start( msg->tranlen, msg->tranadr) )  {
				USB_PRINTF0("### FIFO access error \n");
				usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
			}
			break;
		case USB_SETUPNDC:	// Next stage to Control write no data
			usb_hstd_status_start( );
			break;
		}
		break;
	case USB_INT_SIGN :
		USB_PRINTF0("***SIGN\n");
		g_usb_hstd_ignore_cnt[USB_PIPE0]++;			// Ignore count
		USB_PRINTF2("### IGNORE Pipe %d is %d times (Setup) \n", USB_PIPE0, g_usb_hstd_ignore_cnt[USB_PIPE0]);
		if( USB_PIPEERROR == g_usb_hstd_ignore_cnt[USB_PIPE0] )
			usb_hstd_ctrl_end( USB_DATA_ERR );		// Setup Device Ignore count over
		else  {
			usb_cpu_delay_xms( 5 );				// Wait 5ms
			USB0.INTSTS1.WORD = ~USB_SIGN & INTSTS1_MASK;	// Status Clear
			USB0.INTSTS1.WORD = ~USB_SACK & INTSTS1_MASK;
			USB0.INTENB1.WORD |= USB_SIGNE;			// Setup Ignore,Setup Acknowledge enable
			USB0.INTENB1.WORD |= USB_SACKE;
			USB0.DCPCTR.WORD  |= USB_SUREQ;			// Send SETUP request
		}
		break;
	/***** Processing rootport0 *****/
        case USB_INT_OVRCR:	// Port0 OVCR interrupt function
		USB_PRINTF0("OVCR int port0\n");		// Over-current bit check
		usb_hstd_ovcr_notifiation( );			// OVRCR interrupt disable
		break;
	case USB_INT_ATTCH:	// Port0 ATCH interrupt function
		usb_hstd_attach_process( );
		break;
	case USB_INT_BCHG:
	case USB_INT_DTCH:
		usb_hstd_detach_process( );			// USB detach process
		break;
        case USB_INT_VBINT:
		USB0.INTENB0.WORD &= ~USB_VBSE;			// User program
		break;
        case USB_INT_SOFR :
		USB0.INTENB0.WORD &= ~USB_SOFE;			// User program
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_clr_feature
 Description     : Send ClearFeature command to the connected USB device.
 Arguments       : uint16_t addr	: Device address
                 : uint16_t epnum	: Endpoint number
                 : usb_cb_t complete	: Callback function
 Return value    : none
 ******************************************************************************/
void usb_hstd_clr_feature(uint16_t addr, uint16_t epnum, usb_cb_t complete)
{
usb_utr_t *msg;
	if( 0xFF == epnum )  {			// ClearFeature(Device)
		usb_hstd_clr_stall_request[0] = USB_CLEAR_FEATURE | USB_HOST_TO_DEV | USB_STANDARD | USB_DEVICE;
		usb_hstd_clr_stall_request[1] = USB_DEV_REMOTE_WAKEUP;
		usb_hstd_clr_stall_request[2] = 0x0000;
	}
	else  {					// ClearFeature(endpoint)
		usb_hstd_clr_stall_request[0] = USB_CLEAR_FEATURE | USB_HOST_TO_DEV | USB_STANDARD | USB_ENDPOINT;
		usb_hstd_clr_stall_request[1] = USB_ENDPOINT_HALT;
		usb_hstd_clr_stall_request[2] = epnum;
	}
	usb_hstd_clr_stall_request[3] = 0x0000;
	usb_hstd_clr_stall_request[4] = addr;
	msg = USB_GET_BLK( );			// Get Message Block
	msg->tranadr  = USB_NULL;
	msg->complete = complete;
	msg->tranlen  = 0;
	msg->keyword  = USB_PIPE0;
	msg->setup    = usb_hstd_clr_stall_request;
	msg->segment  = USB_TRAN_END;
	usb_hstd_transfer_start_req( msg );
}

/******************************************************************************
 Function Name   : usb_hstd_clr_stall
 Description     : Clear Stall.
 Arguments       : uint16_t pipe	: Pipe number
                 : usb_cb_t complete	: Callback function
 Return value    : none
 ******************************************************************************/
static void usb_hstd_clr_stall(uint16_t pipe, usb_cb_t complete)
{
uint8_t dir_ep;
uint16_t devsel;
	dir_ep = usb_hstd_pipe_to_epadr( pipe );
	devsel = usb_hstd_get_device_address( pipe );
	usb_hstd_clr_feature( devsel >> USB_DEVADDRBIT, dir_ep, complete );
}

/******************************************************************************
 Function Name   : usb_hstd_clr_stall_result
 Description     : Callback function to notify HCD task that usb_hstd_clr_stall
                   function is completed.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
                 : uint16_t  data1	: Not Use
                 : uint16_t  data2	: Not Use
 Return value    : none
 ******************************************************************************/
static void usb_hstd_clr_stall_result(usb_utr_t *msg, uint16_t data1, uint16_t data2)
{
usb_utr_t *blk;
	blk = USB_GET_BLK( );				// Get Message Block
        blk->msginfo = USB_MSG_HCD_CLR_STALL_RESULT;
        blk->status = msg->status;
	USB_SND_MSG( USB_HCD_MBX, blk );		// Send Message
}

/******************************************************************************
 Function Name   : usb_hstd_hcd_task
 Description     : USB Host Control Driver Task.
 Argument        : usb_utr_t *msg	: Message Address
 Return          : none
 ******************************************************************************/
void usb_hstd_hcd_task(usb_utr_t *msg)
{
uint16_t pipe;
uint16_t connect_inf;
uint16_t retval;
	pipe = msg->keyword;				// Set Parameter
	switch( msg->msginfo )  {			// Branch Hcd Task receive Message Command
	case USB_MSG_HCD_INT:			// USB INT
		usb_hstd_interrupt( msg );
		return;
	case USB_MSG_HCD_SUBMITUTR:		// USB Submit utr
		usb_hstd_set_submitutr( msg );
		if( g_usb_scheduler_block <= msg  && msg <= &g_usb_scheduler_block[USB_BLKMAX-1] )
			break;
		return;
	case USB_MSG_HCD_USBRESET:		// USB Bus Reset
		usb_hstd_bus_reset( );
		connect_inf = usb_cstd_port_speed( );	// Check Current Port Speed
		msg->complete( msg, USB_NULL, connect_inf );
		break;
	case USB_MSG_HCD_VBON:
		usb_hstd_ovrcr_enable( );		// Interrupt Enable
		usb_hstd_vbus_control( USB_VBON );	// USB VBUS control ON
		msg->complete( msg, pipe, USB_MSG_HCD_VBON );
		break;
	case USB_MSG_HCD_VBOFF:
		usb_hstd_vbus_control( USB_VBOFF );	// USB VBUS control OFF
                usb_hstd_ovrcr_disable( );
		usb_cpu_delay_xms( 100 );		// Wait100ms
		msg->complete( msg, pipe, USB_MSG_HCD_VBOFF );
		break;
	case USB_MSG_HCD_CLR_STALL:
                usb_hstd_clr_stall_call = msg->complete;
                usb_hstd_clr_stall_pipe = pipe;
		usb_hstd_clr_stall( pipe, usb_hstd_clr_stall_result );
		break;
	case USB_MSG_HCD_CLR_STALL_RESULT:
		retval = msg->status;
		if( USB_DATA_TMO == retval )  {
			USB_PRINTF0("*** Standard Request Timeout error !\n");
		}
                else if( USB_DATA_STALL == retval )  {
			USB_PRINTF0("*** Standard Request STALL !\n");
		}
                else if( USB_CTRL_END != retval )  {
			USB_PRINTF0("*** Standard Request error !\n");
		}
                else  {
			usb_cstd_clr_stall( usb_hstd_clr_stall_pipe );
			hw_usb_set_sqclr( usb_hstd_clr_stall_pipe );	// SQCLR
		}
                usb_hstd_clr_stall_call( msg, retval, USB_MSG_HCD_CLR_STALL );
		break;
	case USB_MSG_HCD_CLRSEQBIT:
		hw_usb_set_sqclr( USB_PIPE1 );		// SQCLR
		msg->complete( msg, USB_NO_ARG, USB_MSG_HCD_CLRSEQBIT );
		break;
	case USB_MSG_HCD_SETSEQBIT:
		hw_usb_set_sqset( USB_PIPE1 );		// SQSET
		msg->complete( msg, USB_NO_ARG, USB_MSG_HCD_SETSEQBIT );
		break;
	}
	USB_REL_BLK( msg );
}

/******************************************************************************
 Function Name   : usb_hstd_send_start
 Description     : Start data transmission using CPU/DMA transfer to USB host/
                   /device.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_send_start(void)
{
usb_utr_t *msg;
	msg = g_usb_hstd_pipe[USB_PIPE1];				// Evacuation pointer
	if( USB_TRAN_CONT == msg->segment )				// Check transfer count
		usb_hstd_do_sqtgl( USB_PIPE1, msg->pipectr);		// Sequence toggle
	usb_cstd_set_nak( USB_PIPE1 );					// Select NAK
	g_usb_hstd_data_cnt[USB_PIPE1] = msg->tranlen;			// Set data count
	g_usb_hstd_data_ptr[USB_PIPE1] = msg->tranadr;			// Set data pointer
	g_usb_hstd_ignore_cnt[USB_PIPE1] = 0;				// Ignore count clear
	USB0.BEMPSTS.WORD = ~(1 << USB_PIPE1) & BEMPSTS_MASK;		// BEMP Status Clear
	USB0.BRDYSTS.WORD = ~(1 << USB_PIPE1) & BRDYSTS_MASK;		// BRDY Status Clear
	g_usb_cstd_dma_fifo = usb_cstd_get_buf_size( USB_PIPE1 );	// Buffer size
	if( g_usb_hstd_data_cnt[USB_PIPE1] <= g_usb_cstd_dma_fifo )  {	// Check data count
		g_usb_cstd_dma_size = g_usb_hstd_data_cnt[USB_PIPE1];	// Transfer data size
		USB0.BEMPENB.WORD |= 1 << USB_PIPE1;			// Enable Empty Interrupt
	}
	else								// Data size == FIFO size
		g_usb_cstd_dma_size = g_usb_hstd_data_cnt[USB_PIPE1]
		- ( g_usb_hstd_data_cnt[USB_PIPE1] % g_usb_cstd_dma_fifo );
	usb_cstd_dma_snd_start( );
	usb_cstd_set_buf( USB_PIPE1 );					// Set BUF
}

 /******************************************************************************
 Function Name   : usb_hstd_buf_to_fifo
 Description     : Set USB registers as required to write from data buffer to USB 
                   FIFO, to have USB FIFO to write data to bus.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_buf_to_fifo(void)
{
	USB0.BRDYENB.WORD &= ~(1 << USB_PIPE1);			// Disable Ready Interrupt
	g_usb_hstd_ignore_cnt[USB_PIPE1] = 0;			// Ignore count clear
	switch( usb_hstd_write_data( USB_PIPE1, USB_D0USE ) )  {// Check FIFO access sequence
	case USB_WRITING:		// Continue of data write
		USB0.BRDYENB.WORD |= 1 << USB_PIPE1;		// Enable Ready Interrupt
		USB0.NRDYENB.WORD |= 1 << USB_PIPE1;		// Enable Not Ready Interrupt
		break;
	case USB_WRITEEND:		// End of data write
	case USB_WRITESHRT:		// End of data write
		USB0.BEMPENB.WORD |= 1 << USB_PIPE1;		// Enable Empty Interrupt
	        USB0.NRDYENB.WORD |= 1 << USB_PIPE1;		// Enable Not Ready Interrupt
		break;
	case USB_FIFOERROR:		// FIFO access error
		USB_PRINTF0("### FIFO access error \n");
	default:
		usb_hstd_forced_termination( USB_DATA_ERR );
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_driver_registration
 Description     : The HDCD information registered in the class driver structure
                   is registered in HCD.
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_driver_registration(void)
{
	g_usb_hstd_device_state = USB_DETACHED;		// Device state
	USB_PRINTF0("*** Registration driver\n");
}

/******************************************************************************
 Function Name   : usb_hstd_driver_release
 Description     : Release the Device Class Driver.
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_driver_release(void)
{
	g_usb_hstd_device_state = USB_DETACHED;		// Device state
}

/******************************************************************************
 Function Name   : usb_hstd_set_pipe_info
 Description     : Copy information of pipe information table from source
                    (2nd argument) to destination (1st argument)
 Argument        : uint16_t pipe 		: Pipe no
                 : usb_pipe_table_t *dst_ep_tbl	: DEF_EP table pointer(destination)
                 : usb_pipe_table_t *src_ep_tbl	: DEF_EP table pointer(source)
 Return          : none
 ******************************************************************************/
void usb_hstd_set_pipe_info(uint16_t pipe, usb_pipe_table_reg_t *src_ep_tbl)
{
	g_usb_pipe_table.use_flag  = USB_TRUE;
	g_usb_pipe_table.pipe_cfg  = src_ep_tbl->pipe_cfg;
	g_usb_pipe_table.pipe_maxp = src_ep_tbl->pipe_maxp;
}

/******************************************************************************
 Function Name   : usb_hstd_return_enu_mgr
 Description     : Continuous enumeration is requested to MGR task (API for nonOS)
 Arguments       : uint16_t cls_result	: class check result
 Return          : none
 ******************************************************************************/
void usb_hstd_return_enu_mgr(uint16_t cls_result)
{
	g_usb_hstd_check_enu_result = cls_result;
	usb_hstd_mgr_snd_mbx( USB_MSG_MGR_SUBMITRESULT, USB_PIPE0, USB_CTRL_END );
}

/******************************************************************************
 Function Name   : usb_hstd_change_device_state
 Description     : Request to change the status of the connected USB Device.
 Arguments       : usb_cb_t complete	: callback function pointer
                 : uint16_t msginfo	: Message Information
 Return          : none
 ******************************************************************************/
void usb_hstd_change_device_state(usb_cb_t complete, uint16_t msginfo)
{
usb_utr_t *msg;
	switch( msginfo )  {
	case USB_DO_CLR_STALL:			// USB_MSG_HCD_CLR_STALL
		usb_hstd_hcd_snd_mbx( USB_MSG_HCD_CLR_STALL, USB_PIPE1, 0, complete );
		break;
	case USB_DO_SET_SQTGL:			// USB_MSG_HCD_SQTGLBIT
		usb_hstd_hcd_snd_mbx( USB_MSG_HCD_SETSEQBIT, USB_PIPE1, 0, complete );
		break;
	case USB_DO_CLR_SQTGL:			// USB_MSG_HCD_CLRSEQBIT
		usb_hstd_hcd_snd_mbx( USB_MSG_HCD_CLRSEQBIT, USB_PIPE1, 0, complete );
		break;
	default:
		msg = USB_GET_BLK( );		// Get memory pool blk
                USB_PRINTF1("*** msginfo=%d ***\n", msginfo);
                msg->msginfo = msginfo;
                msg->keyword = USB_PIPE1;
                msg->complete = complete;
		USB_SND_MSG( USB_MGR_MBX, msg );
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_hcd_open
 Description     : Start HCD(Host Control Driver) task.
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_hcd_open(void)
{
	USB_PRINTF0("*** Install USB-HCD ***\n");
	usb_cstd_set_task_pri( USB_HCD_TSK, USB_PRI_1 );
	usb_cstd_set_task_pri( USB_HCD_TSK, USB_PRI_0 );
}

/******************************************************************************
 Function Name   : usb_hstd_dummy_function
 Description     : Dummy function.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
                 : uint16_t  data1	: Not used
                 : uint16_t  data2	: Not used
 Return value    : none
 ******************************************************************************/
void usb_hstd_dummy_function(usb_utr_t *msg, uint16_t data1, uint16_t data2)
{
	// None
}

/******************************************************************************
 Function Name   : usb_hstd_transfer_start
 Description     : Send a request for data transfer to HCD (Host Control Driver) 
                   using the specified pipe.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
 Return          : usb_er_t		: USB_OK/USB_QOVR/USB_ERROR
 ******************************************************************************/
usb_er_t usb_hstd_transfer_start(usb_utr_t *msg)
{
	if( USB_PIPE0 == msg->keyword )			// Check enumeration
		if( USB_DEFAULT == g_usb_hstd_mgr_mode )  {
			USB_REL_BLK( msg );		// Release Message Block
			return USB_QOVR;
		}
	return usb_hstd_transfer_start_req( msg );
}
