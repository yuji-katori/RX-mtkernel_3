/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_creg_access.c
 */

#include "r_usb_basic_if.h"
#include "r_usb_bitdefine.h"

static void *hw_usb_get_fifosel_adr(uint16_t pipemode);
/******************************************************************************
 Function Name   : hw_usb_rmw_dvstctr
 Description     : Read-modify-write the specified port's DVSTCTR.
 Arguments       : uint16_t data	: The value to write
                 : uint16_t bitptn	: Bit pattern to read-modify-write
 Return value    : none
 ******************************************************************************/
void hw_usb_rmw_dvstctr(uint16_t data, uint16_t bitptn)
{
uint16_t buf;
	buf = USB0.DVSTCTR0.WORD;
	buf &= ~bitptn;
	buf |= data;
	USB0.DVSTCTR0.WORD = buf;
}

/******************************************************************************
 Function Name   : hw_usb_set_vbout
 Description     : Set specified port's VBOUT-bit in the DVSTCTR register.
                 : (To output a "High" to pin VBOUT.)
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void hw_usb_set_vbout(void)
{
	if( R_USB_VBUS( ) == USB_CFG_HIGH )
		USB0.DVSTCTR0.WORD |=  USB_VBUSEN;
	else
		USB0.DVSTCTR0.WORD &= ~USB_VBUSEN;
}

/******************************************************************************
 Function Name   : hw_usb_clear_vbout
 Description     : Clear specified port's VBOUT-bit in the DVSTCTR register.
                 : (To output a "Low" to pin VBOUT.)
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void hw_usb_clear_vbout(void)
{
	if( R_USB_VBUS( ) == USB_CFG_HIGH )
		USB0.DVSTCTR0.WORD &= ~USB_VBUSEN;
	else
		USB0.DVSTCTR0.WORD |=  USB_VBUSEN;
}

/******************************************************************************
 Function Name   : hw_usb_write_fifo8
 Description     : Data is written to the specified pipemode's FIFO register, 8-bits
                   wide, corresponding to the specified PIPEMODE.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
                 : uint8_t  data	: Setting value
 Return value    : none
 ******************************************************************************/
void hw_usb_write_fifo8(uint16_t pipemode, uint8_t data)
{
// 実際にはPIPE1(D0FIFO)固定のようであるが、念のためにコードを残す
	if( pipemode == USB_CUSE )
		USB0.CFIFO.BYTE.L = data;
	else		// case USB_D0USE:
		USB0.D0FIFO.BYTE.L = data;
}

/******************************************************************************
 Function Name   : hw_usb_get_fifosel_adr
 Description     : Returns the *address* of the FIFOSEL register corresponding to 
                   specified pipemode.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
 Return value    : none
 ******************************************************************************/
static void *hw_usb_get_fifosel_adr(uint16_t pipemode)
{
	if( pipemode == USB_CUSE )
		return (void*)&USB0.CFIFOSEL;
	else		// case USB_D0USE:
		return (void*)&USB0.D0FIFOSEL;
}

/******************************************************************************
 Function Name   : hw_usb_read_fifosel
 Description     : Returns the value of the specified pipemode's FIFOSEL register.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
 Return value    : FIFOSEL content
 ******************************************************************************/
uint16_t hw_usb_read_fifosel(uint16_t pipemode)
{
volatile __evenaccess uint16_t *FIFOSEL;
	FIFOSEL = hw_usb_get_fifosel_adr( pipemode );
	return *FIFOSEL;
}

/******************************************************************************
 Function Name   : hw_usb_rmw_fifosel
 Description     : Data is written to the specified pipemode's FIFOSEL register 
                   (the FIFOSEL corresponding to the specified PIPEMODE), using 
                   read-modify-write.
 Arguments       : uint16_t data	: Setting value
                 : uint16_t bitptn	: bitptn: Bit pattern to read-modify-write
 Return value    : none
 ******************************************************************************/
void hw_usb_rmw_fifosel(uint16_t data, uint16_t bitptn)
{
uint16_t buf;
	buf = USB0.CFIFOSEL.WORD;
	buf &= ~bitptn;
	buf |= data;
	USB0.CFIFOSEL.WORD = buf;
}

/******************************************************************************
 Function Name   : hw_usb_set_mbw
 Description     : Set MBW-bits (CFIFO Port Access Bit Width) of the FIFOSEL cor-
                   responding to the specified PIPEMODE, to select 8 or 16-bit 
                   wide FIFO port access.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
                 : uint16_t data	: Setting value
                 : (data = 0x0400), 32 bit (data = 0x0800) access mode
 Return value    : none
 ******************************************************************************/
void hw_usb_set_mbw(uint16_t pipemode, uint16_t data)
{
volatile __evenaccess uint16_t *FIFOSEL;
	FIFOSEL = hw_usb_get_fifosel_adr( pipemode );
	if( data )
		*FIFOSEL |=  USB_MBW_16;
	else
		*FIFOSEL &= ~USB_MBW_16;
}

/******************************************************************************
 Function Name   : hw_usb_set_curpipe
 Description     : Set pipe to the number given; in the FIFOSEL corresponding 
                   to specified PIPEMODE.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
                 : uint16_t pipe	: Pipe number
 Return value    : none
 ******************************************************************************/
void hw_usb_set_curpipe(uint16_t pipemode, uint16_t pipe)
{
uint16_t buf;
volatile __evenaccess uint16_t *FIFOSEL;
	FIFOSEL = hw_usb_get_fifosel_adr( pipemode );
	buf = *FIFOSEL;
	if( USB_D0USE == pipemode )
		buf &= ~USB_DREQE;
	buf &= ~USB_CURPIPE;
	*FIFOSEL = buf;
	while( *FIFOSEL & USB_CURPIPE )  ;	// Wait Clear CURPIPE
	buf |= pipe;
	*FIFOSEL = buf;
}

/******************************************************************************
 Function Name   : hw_usb_read_fifoctr
 Description     : Returns the value of the FIFOCTR register corresponding to 
                   specified PIPEMODE.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
 Return value    : FIFOCTR content
 ******************************************************************************/
uint16_t hw_usb_read_fifoctr(uint16_t pipemode)
{
	if( pipemode == USB_CUSE )
		return USB0.CFIFOCTR.WORD;
	else		// case USB_D0USE:
		return USB0.D0FIFOCTR.WORD;
}

/******************************************************************************
 Function Name   : hw_usb_set_bval
 Description     : Set BVAL (Buffer Memory Valid Flag) to the number given; in 
                   the FIFOCTR corresponding to the specified PIPEMODE.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
 Return value    : none
 ******************************************************************************/
void hw_usb_set_bval(uint16_t pipemode)
{
	if( pipemode == USB_CUSE )
		USB0.CFIFOCTR.WORD |= USB_BVAL;
	else		// case USB_D0USE:
		USB0.D0FIFOCTR.WORD |= USB_BVAL;
}

/******************************************************************************
 Function Name   : hw_usb_set_bclr
 Description     : Set BCLR (CPU Buffer Clear) to the number given; in the 
                   FIFOCTR corresponding to the specified PIPEMODE.
 Arguments       : uint16_t pipemode	: CUSE/D0DMA
 Return value    : none
 ******************************************************************************/
void hw_usb_set_bclr(uint16_t pipemode)
{
// 実際にはPIPE1(D0FIFO)固定のようであるが、念のためにコードを残す
	if( pipemode == USB_CUSE )
		USB0.CFIFOCTR.WORD = USB_BCLR;
	else		// case USB_D0USE:
		USB0.D0FIFOCTR.WORD = USB_BCLR;
}

/******************************************************************************
 Function Name   : hw_usb_read_pipectr
 Description     : Returns DCPCTR or the specified pipe's PIPECTR register content
                 : The Pipe Control Register returned is determined by the speci-
                   fied pipe number.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : PIPExCTR content
 ******************************************************************************/
uint16_t hw_usb_read_pipectr(uint16_t pipe)
{
	if( USB_PIPE0 == pipe )
		return USB0.DCPCTR.WORD;
	else
		return USB0.PIPE1CTR.WORD;
}

/******************************************************************************
 Function Name   : hw_usb_set_sqclr
 Description     : The SQCLR-bit (Toggle Bit Clear) is set in the specified pipe's 
                   control register. Setting SQSET to 1 makes DATA0 the expected
                   data in the pipe's next transfer.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : none
 ******************************************************************************/
void hw_usb_set_sqclr(uint16_t pipe)
{
// 実際にはPIPE1(D0FIFO)固定のようであるが、念のためにコードを残す
	if( USB_PIPE0 == pipe )
		USB0.DCPCTR.WORD |= USB_SQCLR;
	else
		USB0.PIPE1CTR.WORD |= USB_SQCLR;
}

/******************************************************************************
 Function Name   : hw_usb_set_sqset
 Description     : The SQSET-bit (Toggle Bit Set) is set in the specified pipe's 
                   control register. Setting SQSET to 1 makes DATA1 the expected 
                   data in the next transfer.
 Arguments       : uint16_t pipe	: Pipe number
 Return value    : none
 ******************************************************************************/
void hw_usb_set_sqset(uint16_t pipe)
{
// 実際にはPIPE1(D0FIFO)固定のようであるが、念のためにコードを残す
	if( USB_PIPE0 == pipe )
		USB0.DCPCTR.WORD |= USB_SQSET;
	else
		USB0.PIPE1CTR.WORD |= USB_SQSET;
}

/******************************************************************************
 Function Name   : hw_usb_clear_pid
 Description     : Clear the specified PID-bits of the specified pipe's DCPCTR/
                   PIPECTR register.
 Arguments       : uint16_t pipe	: Pipe number
                 : uint16_t data	: NAK/BUF/STALL - to be cleared
 Return value    : none
 ******************************************************************************/
void hw_usb_clear_pid(uint16_t pipe, uint16_t data)
{
	if( USB_PIPE0 == pipe )
		USB0.DCPCTR.WORD &= ~data;
	else
		USB0.PIPE1CTR.WORD &= ~data;
}
