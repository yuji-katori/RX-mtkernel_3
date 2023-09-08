/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_creg_abs.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"

/******************************************************************************
 Function Name   : usb_cstd_get_buf_size
 Description     : Return buffer size, or max packet size, of specified pipe.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : uint16_t		: FIFO buffer size or max packet size
 ******************************************************************************/
uint16_t usb_cstd_get_buf_size(uint16_t pipe)
{
uint16_t size, buffer;
	if( USB_PIPE0 == pipe )  {
#if !defined(BSP_MCU_RX630)
		buffer = USB0.DCPCFG.WORD;
#else	/* !defined(BSP_MCU_RX630) */
		buffer = 0;
#endif	/* !defined(BSP_MCU_RX630) */
		if( USB_CFG_CNTMDON == (buffer & USB_CNTMDFIELD) )	// Continuation transmit
			size = USB_PIPE0BUF;				// Buffer Size
		else
			size = USB0.DCPMAXP.WORD & USB_MAXP;		// Max Packet Size
	}
	else  {
		USB0.PIPESEL.WORD = USB_PIPE1;				// Pipe select
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
		if( USB_CFG_CNTMDON == (USB0.PIPECFG.WORD & USB_CNTMDFIELD) )
			size = ( ( USB0.PIPEBUF.WORD >> USB_BUFSIZE_BIT ) + 1 ) * USB_PIPEXBUF;
		else
#endif	/* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
		size = USB_MXPS & USB0.PIPEMAXP.WORD;			// Max Packet Size
	}
	return size;
}

/******************************************************************************
 Function Name   : usb_cstd_pipe_init
 Description     : Initialization of registers associated with specified pipe.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_cstd_pipe_init(void)
{
	g_usb_hstd_pipe[USB_PIPE1] = USB_NULL;
	USB0.BRDYENB.WORD &= ~(1 << USB_PIPE1);			// Ready Int Disable
	USB0.NRDYENB.WORD &= ~(1 << USB_PIPE1);			// NotReady Int Disable
	USB0.BEMPENB.WORD &= ~(1 << USB_PIPE1);			// Empty/SizeErr Int Disable
	usb_cstd_clr_stall( USB_PIPE1 );			// PID=NAK & clear STALL
	USB0.PIPESEL.WORD = USB_PIPE1;				// Pipe Select
	g_usb_pipe_table.pipe_cfg |= USB_BFREON;
	USB0.PIPECFG.WORD  = g_usb_pipe_table.pipe_cfg;
	USB0.PIPEMAXP.WORD = g_usb_pipe_table.pipe_maxp;
	USB0.PIPESEL.WORD = 0;					// FIFO buffer DATA-PID initialized
	USB0.PIPE1CTR.WORD |=  USB_SQCLR;
	USB0.PIPE1CTR.WORD &= ~USB_ACLRM;
	USB0.BRDYSTS.WORD = ~(1 << USB_PIPE1) & BRDYSTS_MASK;	// Ready         Int Clear
	USB0.NRDYSTS.WORD = ~(1 << USB_PIPE1) & NRDYSTS_MASK;	// NotReady      Int Clear
	USB0.BEMPSTS.WORD = ~(1 << USB_PIPE1) & BEMPSTS_MASK;	// Empty/SizeErr Int Clear
}

/******************************************************************************
 Function Name   : usb_cstd_clr_pipe_cnfg
 Description     : Clear specified pipe configuration register.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_cstd_clr_pipe_cnfg(void)
{
	g_usb_hstd_pipe[USB_PIPE1] = USB_NULL;
	usb_cstd_clr_stall( USB_PIPE1 );			// PID=NAK & clear STALL
	USB0.BRDYENB.WORD &= ~(1 << USB_PIPE1);			// Ready         Int Disable
	USB0.NRDYENB.WORD &= ~(1 << USB_PIPE1);			// NotReady      Int Disable
	USB0.BEMPENB.WORD &= ~(1 << USB_PIPE1);			// Empty/SizeErr Int Disable
	usb_cstd_chg_curpipe( USB_PIPE0, USB_CUSE, USB_FALSE );	// PIPE Configuration
	USB0.PIPESEL.WORD = USB_PIPE1;
	USB0.PIPECFG.WORD = 0;
	USB0.PIPEMAXP.WORD = 0;
	USB0.PIPEPERI.WORD = 0;
	USB0.PIPESEL.WORD  = 0;					// FIFO buffer DATA-PID initialized
	USB0.PIPE1CTR.WORD |=  USB_SQCLR;
	USB0.PIPE1CTR.WORD &= ~USB_ACLRM;
	USB0.PIPE1TRE.WORD &= ~USB_TRENB;
	USB0.PIPE1TRE.WORD |=  USB_TRCLR;
	USB0.BRDYSTS.WORD = ~(1 << USB_PIPE1) & BRDYSTS_MASK;	// Ready         Int Clear
	USB0.NRDYSTS.WORD = ~(1 << USB_PIPE1) & NRDYSTS_MASK;	// NotReady      Int Clear
	USB0.BEMPSTS.WORD = ~(1 << USB_PIPE1) & BEMPSTS_MASK;	// Empty/SizeErr Int Clear
}

/******************************************************************************
 Function Name   : usb_cstd_set_nak
 Description     : Set up to NAK the specified pipe.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : none
 ******************************************************************************/
void usb_cstd_set_nak(uint16_t pipe)
{
int i;
	hw_usb_clear_pid( pipe, USB_PID_BUF );				// Set NAK
	// The state of PBUSY continues while transmitting the packet when it is a detach.
	// 1ms comes off when leaving because the packet duration might not exceed 1ms.
	// Whether it is PBUSY release or 1ms passage can be judged.
	for( i=0 ; i<0xFFFF ; i++ )
		if( !( hw_usb_read_pipectr( pipe ) & USB_PBUSY ) )	// PIPE control reg read
			break;
}

/******************************************************************************
 Function Name   : usb_cstd_is_set_frdy
 Description     : Changes the specified FIFO port by the specified pipe.
 Arguments       : uint16_t pipe	: Pipe number
                 : uint16_t pipemode	: CUSE/D0DMA
                 : uint16_t isel	: ISEL bit status
 Return value    : FRDY status
 ******************************************************************************/
uint16_t usb_cstd_is_set_frdy(uint16_t pipe, uint16_t pipemode, uint16_t isel)
{
uint16_t buffer;
int i;
	usb_cstd_chg_curpipe( pipe, pipemode, isel );		// Changes the FIFO port by the pipe
	for( i=0 ; i<4000 ; i++ )  {
		buffer = hw_usb_read_fifoctr( pipemode );
		if( USB_FRDY == ( buffer & USB_FRDY ) )
			return buffer;
		USB_PRINTF1("*** FRDY wait pipe = %d\n", pipe);
		// Caution!!!
		// Depending on the external bus speed of CPU, you may need to wait for 100ns here.
		// For details, please look at the data sheet.
		/***** The example of reference. *****/
		buffer = USB0.SYSCFG.WORD;
		buffer = USB0.SYSSTS0.WORD;
		/*************************************/
	}
	return USB_FIFOERROR;
}

/******************************************************************************
 Function Name   : usb_cstd_chg_curpipe
 Description     : Switch FIFO and pipe number.
 Arguments       : uint16_t pipe	: Pipe number
                 : uint16_t pipemode	: CUSE/D0DMA
                 : uint16_t isel	: CFIFO Port Access Direction.(Pipe1 to 9:Set to 0)
 Return value    : none
 ******************************************************************************/
void usb_cstd_chg_curpipe(uint16_t pipe, uint16_t pipemode, uint16_t isel)
{
	if( pipemode == USB_CUSE )  {			// CFIFO use
		hw_usb_rmw_fifosel( USB_RCNT | isel | pipe, USB_RCNT | USB_ISEL | USB_CURPIPE );
		while( ( hw_usb_read_fifosel( USB_CUSE ) & (USB_ISEL | USB_CURPIPE) ) != isel | pipe )  ;
	}
	else  {						// D0FIFO use
		hw_usb_set_curpipe( USB_D0USE, pipe );	// DxFIFO pipe select
		while( ( hw_usb_read_fifosel( USB_D0USE ) & USB_CURPIPE ) != pipe )  ;
	}
}

/******************************************************************************
 Function Name   : usb_cstd_clr_transaction_counter
 Description     : Clear specified Pipe Transaction Counter Register.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : none
 ******************************************************************************/
void usb_cstd_clr_transaction_counter(uint16_t pipe)
{
	USB0.PIPE1TRE.WORD &= ~USB_TRENB;
	USB0.PIPE1TRE.WORD |=  USB_TRCLR;
}
