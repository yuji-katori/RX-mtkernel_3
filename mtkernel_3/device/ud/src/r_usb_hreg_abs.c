/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hreg_abs.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"

/******************************************************************************
 Function Name   : usb_hstd_interrupt_handler
 Description     : Analyzes which USB interrupt is generated.
 Arguments       : usb_utr_t  *msg	: Pointer to usb_utr_t structure
 Return          : none
 ******************************************************************************/
void usb_hstd_interrupt_handler(usb_utr_t *msg)
{
uint16_t ists0 = USB0.INTSTS0.WORD & USB0.INTENB0.WORD;
uint16_t ists1 = USB0.INTSTS1.WORD & USB0.INTENB1.WORD;
uint16_t bsts  = USB0.BRDYSTS.WORD & USB0.BRDYENB.WORD;
uint16_t nsts  = USB0.NRDYSTS.WORD & USB0.BRDYENB.WORD;
uint16_t ests  = USB0.BEMPSTS.WORD & USB0.BEMPENB.WORD;

	msg->keyword = USB_INT_UNKNOWN;				// Interrupt Status Get
	msg->status = 0;
	/***** Processing Setup transaction *****/
	if( USB_SACK == ( ists1 & USB_SACK ) )  {		// ***** Setup ACK *****
		USB0.INTSTS1.WORD  = ~USB_SACK & INTSTS1_MASK;	// SACK Clear
		USB0.INTENB1.WORD &= ~( USB_SIGNE | USB_SACKE );// Setup Ignore,Setup Acknowledge disable
		msg->keyword = USB_INT_SACK;
	}
	else if( USB_SIGN == ( ists1 & USB_SIGN ) )  {		// ***** Setup Ignore *****
		USB0.INTSTS1.WORD  = ~USB_SIGN & INTSTS1_MASK;	// SIGN Clear
		USB0.INTENB1.WORD &= ~(USB_SIGNE | USB_SACKE);	// Setup Ignore,Setup Acknowledge disable
		msg->keyword = USB_INT_SIGN;
	}
	else if( USB_BRDY == ( ists0 & USB_BRDY ) )  {		// ***** Processing PIPE0-MAX_PIPE_NO data *****
		USB0.BRDYSTS.WORD = ~bsts & BRDYSTS_MASK;
		msg->keyword = USB_INT_BRDY;
		msg->status = bsts;
	}
	else if( USB_BEMP == ( ists0 & USB_BEMP ) )  {		// ***** EP0-7 BEMP *****
		USB0.BEMPSTS.WORD = ~ests & BEMPSTS_MASK;
		msg->keyword = USB_INT_BEMP;
		msg->status = ests;
	}
	else if( USB_NRDY == ( ists0 & USB_NRDY ) )  {		// ***** EP0-7 NRDY *****
		USB0.NRDYSTS.WORD = ~nsts & NRDYSTS_MASK;
		msg->keyword = USB_INT_NRDY;
		msg->status = nsts;
	}
	else if( USB_OVRCR == ( ists1 & USB_OVRCR ) )  {	// ***** OVER CURRENT *****
		USB0.INTSTS1.WORD = ~USB_OVRCR & INTSTS1_MASK;	// OVRCR Clear
		msg->keyword = USB_INT_OVRCR;
	}
	else if( USB_ATTCH == ( ists1 & USB_ATTCH ) )  {	// ***** ATTCH INT *****
		usb_hstd_bus_int_disable( );			// DTCH  interrupt disable
		msg->keyword = USB_INT_ATTCH;
	}
	else if( USB_EOFERR == ( ists1 & USB_EOFERR ) )  { 	// ***** EOFERR INT *****
		USB0.INTSTS1.WORD = ~USB_EOFERR & INTSTS1_MASK;	// EOFERR Clear
		msg->keyword = USB_INT_EOFERR;
	}
	else if( USB_BCHG == ( ists1 & USB_BCHG ) )  {		// ***** BCHG INT *****
		usb_hstd_bchg_disable( );			// BCHG  interrupt disable
		msg->keyword = USB_INT_BCHG;
	}
	else if( USB_DTCH == ( ists1 & USB_DTCH ) )  {		// ***** DETACH *****
		usb_hstd_bus_int_disable( );			// DTCH  interrupt disable
		msg->keyword = USB_INT_DTCH;
	}
	else if( USB_VBINT == ( ists0 & USB_VBINT ) )  {	// ***** VBUS change *****
		USB0.INTSTS0.WORD = ~USB_VBINT & 0xFFFF;	// Status Clear
		msg->keyword = USB_INT_VBINT;
	}
	else if( USB_SOFR == ( ists0 & USB_SOFR ) )  {		// ***** SOFR change *****
		USB0.INTSTS0.WORD = ~USB_SOFR & 0xFFFF;		// SOFR Clear
		msg->keyword = USB_INT_SOFR;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_chk_attach
 Description     : Checks whether USB Device is attached or not and return USB speed
                   of USB Device.
 Arguments       : none
 Return value    : uint16_t		: connection status
                 :			: (USB_ATTACHF/USB_ATTACHL/USB_DETACH/USB_OK)
 Note            : Please change for your SYSTEM
 ******************************************************************************/
uint16_t usb_hstd_chk_attach(void)
{
uint16_t buf[2];

	do  {
		buf[0] = USB0.SYSSTS0.WORD;
		usb_cpu_delay_xms( 30 );		// Wait 30ms
		buf[1] = USB0.SYSSTS0.WORD;
		if( ( buf[0] & USB_LNST ) == ( buf[1] & USB_LNST ) )  {
			usb_cpu_delay_xms( 20 );	// Wait 20ms
			buf[1] = USB0.SYSSTS0.WORD;
		}
	} while( ( buf[0] & USB_LNST ) != ( buf[1] & USB_LNST ) )  ;
	buf[1] = USB0.DVSTCTR0.WORD;
	if( USB_UNDECID == ( buf[1] & USB_RHST ) )  {
		if( USB_FS_JSTS == ( buf[0] & USB_LNST ) )  {
			USB_PRINTF0(" Detect FS-J\n");		// High/Full speed device
			return USB_ATTACHF;
		}
		else if( USB_LS_JSTS == ( buf[0] & USB_LNST ) )  {
			USB_PRINTF0(" Attach LS device\n");	// Low speed device
			return USB_ATTACHL;
		}
		else if( USB_SE0 == ( buf[0] & USB_LNST ) )  {
			USB_PRINTF0(" Detach device\n");
		}
		else  {
			USB_PRINTF0(" Attach unknown speed device\n");
		}
	}
	else  {
		USB_PRINTF0(" Already device attached\n");
		return USB_OK;
	}
	return USB_DETACH;
}

/******************************************************************************
 Function Name   : usb_hstd_chk_clk
 Description     : Checks SOF sending setting when USB Device is detached or suspended
                   , BCHG interrupt enable setting and clock stop processing.
 Arguments       : uint16_t event	: device state
 Return value    : none
 ******************************************************************************/
void usb_hstd_chk_clk(uint16_t event)
{
	if( USB_DETACHED == g_usb_hstd_mgr_mode /*|| USB_SUSPENDED == g_usb_hstd_mgr_mode*/ )  {
		usb_cpu_delay_1us( 1 );			// Wait 640ns
		usb_hstd_bchg_enable( );		// Enable port BCHG interrupt
	}
}

/******************************************************************************
 Function Name   : usb_hstd_detach_process
 Description     : Handles the require processing when USB device is detached.
                   (Data transfer forcibly termination processing to the connected USB Device,
                   the clock supply stop setting and the USB interrupt dissable setteing etc)
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_detach_process(void)
{
uint16_t connect_inf;
uint16_t devsel;

	usb_hstd_attch_disable( );		// ATTCH interrupt disable
	usb_hstd_dtch_disable( );		// DTCH  interrupt disable
	usb_hstd_bchg_disable( );		// BCHG  interrupt disable
	devsel = USB_DEVICE1 << USB_DEVADDRBIT;
	if( usb_hstd_chk_dev_addr( devsel ) != USB_NOCONNECT )  {
		if( USB_IDLEST != g_usb_hstd_ctsq )
			usb_hstd_ctrl_end( USB_DATA_ERR );	// Control Read/Write End
		if( usb_hstd_get_devsel( ) == devsel )  {	// Agreement device address
			if( USB_PID_BUF == usb_cstd_get_pid( USB_PIPE1 ) )	// PID=BUF ?
				usb_hstd_forced_termination( USB_DATA_STOP );
			usb_cstd_clr_pipe_cnfg(  );		// End of data transfer (IN/OUT)
		}
		usb_hstd_set_dev_addr( devsel, USB_NOCONNECT );
		USB_PRINTF0("*** Device address clear\n");
	}
	connect_inf = usb_hstd_chk_attach( );		// Decide USB Line state (ATTACH)
	switch( connect_inf )  {
	case USB_ATTACHL:
        case USB_ATTACHF:
		usb_hstd_attach( connect_inf );
		break;
	case USB_DETACH:
	default:
		usb_hstd_detach( );			// USB detach
		usb_hstd_chk_clk( USB_DETACHED );	// Check clock
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_attach_process
 Description     : Interrupt disable setting when USB Device is attached and
                   handles the required interrupt disable setting etc when USB device
                   is attached.
 Arguments       : none
 Return value    : none
 Note            : Please change for your SYSTEM
 ******************************************************************************/
void usb_hstd_attach_process(void)
{
uint16_t connect_inf;

	usb_hstd_attch_disable( );			// ATTCH interrupt disable
	usb_hstd_dtch_disable( );			// DTCH  interrupt disable
	usb_hstd_bchg_disable( );			// BCHG  interrupt disable
	connect_inf = usb_hstd_chk_attach( );		// Decide USB Line state (ATTACH)
	switch( connect_inf )  {
	case USB_ATTACHL:
        case USB_ATTACHF:
		usb_hstd_attach( connect_inf );
		break;
        case USB_DETACH :
		usb_hstd_detach( );			// USB detach
		usb_hstd_chk_clk( USB_DETACHED );	// Check clock
		break;
	default:
		usb_hstd_attach( USB_ATTACHF );
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_bus_reset
 Description     : Setting USB register when BUS Reset.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_bus_reset(void)
{
int i;
uint16_t buf;
	hw_usb_rmw_dvstctr( USB_USBRST, USB_USBRST | USB_UACT );	// USBRST=1, UACT=0
	usb_cpu_delay_xms( 50 );			// Wait 50ms
	usb_hstd_set_uact( );				// USBRST=0, RESUME=0, UACT=1
	usb_cpu_delay_xms( 20 );			// Wait 10ms or more (USB reset recovery)
	for( i=0, buf=USB_HSPROC; i<3 && USB_HSPROC==buf ; i++ )  {
		buf = USB0.DVSTCTR0.WORD & USB_RHST;	// DeviceStateControlRegister - ResetHandshakeStatusCheck
		if( USB_HSPROC == buf )
			usb_cpu_delay_xms( 10 );	// Wait 10ms
	}
	usb_cpu_delay_xms( 30 );			// Wait 30ms
}

/******************************************************************************
 Function Name   : usb_hstd_support_speed_check
 Description     : Get USB-speed of the specified port.
 Arguments       : none
 Return value    : uint16_t		: HSCONNECT : Hi-Speed
                 :			: FSCONNECT : Full-Speed
                 :			: LSCONNECT : Low-Speed
                 :			: NOCONNECT : not connect
 ******************************************************************************/
uint16_t usb_hstd_support_speed_check(void)
{
uint16_t conn_inf;

	switch( USB0.DVSTCTR0.WORD & USB_RHST )  {	// Get port speed
	case USB_HSMODE:
		conn_inf = USB_HSCONNECT;
		break;
	case USB_FSMODE:
		conn_inf = USB_FSCONNECT;
		break;
	case USB_LSMODE:
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671)
		conn_inf = USB_LSCONNECT;
#else   /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
		conn_inf = USB_NOCONNECT;
#endif  /* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
		break;
	case USB_HSPROC:
		conn_inf = USB_NOCONNECT;
		break;
	default:
		conn_inf = USB_NOCONNECT;
		break;
	}
	return conn_inf;
}

/******************************************************************************
 Function Name   : usb_hstd_write_fifo
 Description     : Write specified amount of data to specified USB FIFO. 
 Arguments       : uint16_t count	: Write size
                 : uint16_t pipemode	: CUSE/D0DMA
                 : uint16_t *write	: Address of buffer of data to write.
 Return value    : The incremented address of last argument (write).
 ******************************************************************************/
uint8_t *usb_hstd_write_fifo(uint16_t count, uint16_t pipemode, uint8_t *write)
{
//uint16_t i;
//	for( i=1 ; i<count ; i+=2 )  {
//		hw_usb_write_fifo16( pipemode, *(uint16_t *)write );	// 16bit access
//		write += sizeof(uint16_t);				// Renewal write pointer
//	}
	// PIPE0‚ÍŠï”ƒTƒCƒY‚É‚È‚é‚±‚Æ‚ª‚È‚¢
	if( count % 2 )  {	// count == odd
		hw_usb_set_mbw( pipemode, USB_MBW_8 );		// Change FIFO access width
		hw_usb_write_fifo8( pipemode, *write++ );	// FIFO write
		hw_usb_set_mbw( pipemode, USB_MBW_16 );		// Return FIFO access width
	}
	return write;
}

/******************************************************************************
 Function Name   : usb_hstd_read_fifo
 Description     : Read specified buffer size from the USB FIFO.
 Arguments       : uint16_t  count	: Read size
                 : uint16_t  read	: Address of buffer to store the read data
 Return value    : Pointer to a buffer that contains the data to be read next
 ******************************************************************************/
uint8_t *usb_hstd_read_fifo(uint16_t count, uint8_t *read)
{
uint16_t i;
uint16_t odd_byte_data_temp;
	for( i=1 ; i<count ; i+=2 )  {
		*(uint16_t *)read = USB0.CFIFO.WORD;	// 16bit FIFO access
		read += sizeof(uint16_t);		// Renewal read pointer
	}
	if( count % 2 )  {
		odd_byte_data_temp = USB0.CFIFO.WORD;	// 16bit FIFO access
	// Condition compilation by the difference of the little endian
#if USB_CFG_ENDIAN == USB_CFG_LITTLE
		*read = odd_byte_data_temp & 0x00FF;
#else	/* USB_CFG_ENDIAN == USB_CFG_LITTLE */
		*read = odd_byte_data_temp >> 8;
#endif	/* USB_CFG_ENDIAN == USB_CFG_LITTLE */
		read += sizeof(uint8_t);				// Renewal read pointer
	}
	return read;
}

/******************************************************************************
 Function Name   : usb_hstd_forced_termination
 Description     : Terminate data transmission and reception.
 Arguments       : uint16_t status	: Transfer status type
 Return value    : none
 Note            : In the case of timeout status, it does not call back.
 ******************************************************************************/
void usb_hstd_forced_termination(uint16_t status)
{
uint16_t buffer;
	usb_cstd_set_nak( USB_PIPE1 );			// Set NAK
	USB0.BRDYENB.WORD &= ~(1 << USB_PIPE1);		// Disable Ready Interrupt
	USB0.NRDYENB.WORD &= ~(1 << USB_PIPE1);		// Disable Not Ready Interrupt
	USB0.BEMPENB.WORD &= ~(1 << USB_PIPE1);		// Disable Empty Interrupt
	usb_cstd_clr_transaction_counter( USB_PIPE1 );
	buffer = hw_usb_read_fifosel( USB_CUSE );			// Clear CFIFO-port
	if( ( buffer & USB_CURPIPE ) == USB_PIPE1 )
		usb_cstd_chg_curpipe( USB_PIPE0, USB_CUSE, USB_FALSE );	// Changes the FIFO port by the pipe
	buffer = hw_usb_read_fifosel( USB_D0USE );			// Clear D0FIFO-port
	if( ( buffer & USB_CURPIPE ) == USB_PIPE1 )
		usb_cstd_chg_curpipe( USB_PIPE0, USB_D0USE, USB_FALSE );// Changes the FIFO port by the pipe
	USB0.PIPE1CTR.WORD &= ~USB_ACLRM;
	if( USB_NULL != g_usb_hstd_pipe[USB_PIPE1] )  {	// Transfer information set
		g_usb_hstd_pipe[USB_PIPE1]->tranlen = g_usb_hstd_data_cnt[USB_PIPE1];
		g_usb_hstd_pipe[USB_PIPE1]->status  = status;
		g_usb_hstd_pipe[USB_PIPE1]->pipectr = hw_usb_read_pipectr( USB_PIPE1 );
		g_usb_hstd_pipe[USB_PIPE1]->errcnt  = g_usb_hstd_ignore_cnt[USB_PIPE1];
		if( USB_NULL != g_usb_hstd_pipe[USB_PIPE1]->complete )
			g_usb_hstd_pipe[USB_PIPE1]->complete( g_usb_hstd_pipe[USB_PIPE1], 0, 0 );
		g_usb_hstd_pipe[USB_PIPE1] = USB_NULL;
	}
}

/******************************************************************************
 Function Name   : usb_hstd_nrdy_endprocess
 Description     : NRDY interrupt processing. (Forced termination of data trans-
                   mission and reception of specified pipe)
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hstd_nrdy_endprocess(void)
{
uint16_t buffer;
	buffer = usb_cstd_get_pid( USB_PIPE1 );
	if( USB_PID_STALL == ( buffer & USB_PID_STALL ) )  {	// STALL ?
		USB_PRINTF0("### STALL Pipe\n");
		usb_hstd_forced_termination( USB_DATA_STALL );	// End of data transfer
	}
	else  {
		buffer = USB0.SYSSTS0.WORD;		// Wait for About 60ns
		g_usb_hstd_ignore_cnt[USB_PIPE1]++;
		USB_PRINTF2("### IGNORE Pipe %d is %d times \n", pipe, g_usb_hstd_ignore_cnt[USB_PIPE1]);
		if( USB_PIPEERROR == g_usb_hstd_ignore_cnt[USB_PIPE1] )  {
			// Data Device Ignore X 3 call back
			// End of data transfer
			usb_hstd_forced_termination( USB_DATA_ERR );
		}
		else  {
			usb_cpu_delay_xms( 5 );		// Wait 5ms
			usb_cstd_set_buf( USB_PIPE1 );	// PIPEx Data Retry
		}
	}
}
