/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_dma.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "r_usb_reg_access.h"
#include "r_usb_dmac.h"

       uint32_t g_usb_cstd_dma_size;		// DMA buffer size
       uint16_t g_usb_cstd_dma_fifo;		// DMA FIFO buffer size

static uint8_t    usb_cstd_dma_fraction_size;	// fraction size(1-3)
static uint8_t   *usb_cstd_dma_fraction_adr;	// fraction data address
static void * const fifo_address[] = {
	0, (void*)&USB0.D0FIFO.WORD, (void*)&USB0.D0FIFO.BYTE.L };	// USB0 D0FIFO address

static void  usb_cstd_dma_rcv_setting(uint8_t *buf, uint32_t size);
static void  usb_cstd_dma_snd_setting(uint8_t *buf, uint32_t size);
static void  usb_cstd_dma_snd_restart(uint8_t *buf, uint32_t size);

/******************************************************************************
Function Name   : usb_cstd_dma_snd_start
Description     : Start transfer using DMA. If transfer size is 0, write
                  more data to buffer.
Arguments       : none
Return value    : none
******************************************************************************/
void usb_cstd_dma_snd_start(void)
{
uint32_t size;
uint8_t  *buf;
	buf = g_usb_hstd_data_ptr[USB_PIPE1];
        size = g_usb_cstd_dma_size & ~USB_BIT_MBW16;
	if( size )  {
		USB0.D0FIFOSEL.WORD &= ~USB_DREQE;			// DMA Transfer request disable
		IR( USB0, D0FIFO0 ) = 0;				// Clear Interrupt Flag
		if( size >= g_usb_cstd_dma_fifo )
			size = (size - (size % g_usb_cstd_dma_fifo));	// Fifo size block transfer
		else  {							// fraction size(1-3)
			usb_cstd_dma_fraction_size = g_usb_cstd_dma_size & USB_BIT_MBW16;
			usb_cstd_dma_fraction_adr  = buf + size;	// fraction data address
		}
		g_usb_cstd_dma_size = size;
		usb_cstd_dma_snd_setting( buf, size );
		usb_cstd_chg_curpipe( USB_PIPE1, USB_D0USE, USB_FALSE );// Changes the FIFO port by the pipe
        	USB0.NRDYENB.WORD |= 1 << USB_PIPE1;			// Enable Not Ready Interrupt
		USB0.D0FIFOSEL.WORD |=  USB_DREQE;			// Set DREQ enable
	}
	else
		usb_hstd_buf_to_fifo( );
}

/******************************************************************************
Function Name   : usb_cstd_dma_rcv_start
Description     : Start transfer using DMA. If transfer size is 0, clear DMA.
Arguments       : none
Return value    : none
******************************************************************************/
void usb_cstd_dma_rcv_start(void)
{
uint16_t mxps;
uint8_t  *buf;
	buf = g_usb_hstd_data_ptr[USB_PIPE1];
	if( g_usb_cstd_dma_size )  {					// Data size check
		USB0.D0FIFOSEL.WORD &= ~USB_DREQE;			// DMA Transfer request disable
		IR( USB0, D0FIFO0 ) = 0;				// Clear Interrupt Flag
		usb_cstd_dma_rcv_setting( buf, g_usb_cstd_dma_size );
		usb_cstd_chg_curpipe( USB_PIPE1, USB_D0USE, USB_FALSE );// Changes the FIFO port by the pipe
		mxps = usb_cstd_get_maxpacket_size( USB_PIPE1 );	// Max Packet Size
		USB0.PIPE1TRE.WORD |= USB_TRCLR;
		USB0.PIPE1TRN = ( g_usb_cstd_dma_size - 1 ) / mxps + 1;	// Set Transaction counter
		USB0.PIPE1TRE.WORD |= USB_TRENB;
		USB0.BRDYENB.WORD |= 1 << USB_PIPE1;			// Enable Ready Interrupt
	        USB0.NRDYENB.WORD |= 1 << USB_PIPE1;			// Enable Not Ready Interrupt
		USB0.D0FIFOSEL.WORD &= ~USB_DCLRM;			// usb fifo set automatic clear mode
		USB0.D0FIFOSEL.WORD |=  USB_DREQE;			// Set DREQ enable
		usb_cstd_set_buf( USB_PIPE1 );				// Set BUF
	}
}

/******************************************************************************
Function Name   : usb_cstd_dma_snd_restart
Description     : Start transfer using DMA0. accsess size 32bytes.
Arguments       : uint8_t  *buf		: transfer buffer pointer
                : uint32_t  size	: transfer data size
Return value    : none
******************************************************************************/
static void usb_cstd_dma_snd_restart(uint8_t *buf, uint32_t size)
{
	usb_cstd_chg_curpipe( USB_PIPE1, USB_D0USE, USB_FALSE );	// Changes the FIFO port by the pipe
	USB0.D0FIFOSEL.WORD &= ~USB_DREQE;				// DMA Transfer request disable
	IR( USB0, D0FIFO0 ) = 0;					// Clear Interrupt Flag
	usb_cstd_dma_snd_setting( buf, size );				// dma trans setting Divisible by FIFO buffer size
	USB0.NRDYENB.WORD |= 1 << USB_PIPE1;				// Enable Not Ready Interrupt
	USB0.D0FIFOSEL.WORD |=  USB_DREQE;				// Set DREQ enable
}

/******************************************************************************
Function Name   : usb_cstd_dma_driver
Description     : USB DMA transfer complete process.
Arguments       : none
Return value    : none
******************************************************************************/
void usb_cstd_dma_driver(void)
{
uint8_t  *buf;
uint32_t  size;
bool cpu_write = USB_FALSE;
	if( g_usb_hstd_data_cnt[USB_PIPE1] < g_usb_cstd_dma_fifo )
		if( usb_cstd_dma_fraction_size > 0 )				// fraction size(1-3)
			cpu_write = USB_TRUE;					// Set flag for CPU FIFO Write
		else  {
			USB0.BEMPSTS.WORD = ~(1 << USB_PIPE1) & BEMPSTS_MASK;	// FIFO buffer empty flag clear
			USB0.D0FIFOCTR.WORD |= USB_BVAL;			// bval control for transfer enable
			USB0.BEMPENB.WORD |= 1 << USB_PIPE1;			// FIFO bufer empty interrupt enable
		}
	else  {
		g_usb_hstd_data_cnt[USB_PIPE1] -= g_usb_cstd_dma_size;		// update remaining transfer data size
		if( ! g_usb_hstd_data_cnt[USB_PIPE1] )  {			// check transfer remaining data
			USB0.BEMPSTS.WORD = ~(1 << USB_PIPE1) & BEMPSTS_MASK;	// FIFO buffer empty flag clear
			if( ( hw_usb_read_pipectr( USB_PIPE1 ) & USB_INBUFM ) != USB_INBUFM )	// Check FIFO_EMPTY
				usb_hstd_data_end( USB_DATA_NONE );		// DMA transfer function end. call callback function
			else
				USB0.BEMPENB.WORD |= 1 << USB_PIPE1;		// FIFO bufer empty interrupt enable
		}
		else  {
			buf = g_usb_hstd_data_ptr[USB_PIPE1] + g_usb_cstd_dma_size;
			size = g_usb_cstd_dma_size = g_usb_hstd_data_cnt[USB_PIPE1];		// DMA Transfer size update
			usb_cstd_dma_fraction_size = g_usb_cstd_dma_size & USB_BIT_MBW16;	// fraction size(1-3)
        	        size &= ~USB_BIT_MBW16;
			usb_cstd_dma_fraction_adr  = buf + size;		// fraction data address
			if( size )  {
				g_usb_cstd_dma_size = size;
				usb_cstd_dma_snd_restart( buf, size );		// DMA0 1byte trans
			}
			else
				cpu_write = USB_TRUE;				// Set flag for CPU FIFO Write
		}
	}
	if( USB_TRUE == cpu_write )  {
		g_usb_hstd_data_cnt[USB_PIPE1] = usb_cstd_dma_fraction_size;	// fraction size(1-3)
		g_usb_hstd_data_ptr[USB_PIPE1] = usb_cstd_dma_fraction_adr;	// fraction data address
		usb_hstd_buf_to_fifo( );
		usb_cstd_dma_fraction_size = 0;
	}
}

/******************************************************************************
Function Name   : DMA_End_hdr
Description     : Set event for DMA transfer complete of Buffer to DxFIFO.
Arguments       : none
Return value    : none
******************************************************************************/
void DMA_End_hdr(void)
{
        usb_cstd_dma_stop( );			// Stop DMA,FIFO access
	USB0.D0FIFOSEL.WORD &= ~USB_DREQE;	// DMA Transfer request disable
	USB_DmaCompleteEvent( );		// Set USB Event(DMA Complete)
}

/******************************************************************************
Function Name   : usb_cstd_dma_rcv_setting
Description     : FIFO to Buffer data read DMA start.
Arguments       : uint8_t  *buf		: transfer buffer pointer
                : uint32_t  size	: transfer data size
Return value    : void
******************************************************************************/
static void usb_cstd_dma_rcv_setting(uint8_t *buf, uint32_t size)
{
volatile struct st_dmac1 __evenaccess *DMACA;
uint16_t block_size;
extern uint8_t g_usb_hmsc_cbw[], g_usb_hmsc_csw[];
	DMACA = R_DMACA_Address( );				// Set DMAC Channel Address
	DMACA->DMSTS.BIT.DTIF = 0;				// Clear Transfer Interrupt Flag
	if( buf == g_usb_hmsc_cbw || buf == g_usb_hmsc_csw )	// Check Buffer Address
		block_size = ( size + 1 ) / 2;			// Block Size is Buffer Size
	else
		block_size = ( g_usb_cstd_dma_fifo + 1 ) / 2;	// Block Size is FIFO Size
	(&ICU.DMRSR0)[R_DMACA_Channel( )<<2] = VECT( USB0, D0FIFO0 );	// Set DMRSR
    	DMACA->DMAMD.WORD = 0x0080;					// Set DMAMD
	DMACA->DMTMD.WORD = 0xA101;					// Set DMTMD
	DMACA->DMSAR = fifo_address[USB_FIFO_ACCESS_TYPE_16BIT];	// Set DMSAR
	DMACA->DMDAR = buf;						// Set DMSAR
	DMACA->DMCRA = block_size << 16 | block_size;			// Set DMCRA
	DMACA->DMCRB = ( size - 1 ) / g_usb_cstd_dma_fifo + 1;		// Set DMCRB
	DMACA->DMINT.BYTE = 0x00;					// Set DMINT
	R_DMACA_Int_Enable( );
	DMACA->DMCNT.BYTE = 0x01;				// Enable Transfer
}

/******************************************************************************
Function Name   : usb_cstd_dma_snd_setting
Description     : Buffer to FIFO data write DMA start.
Arguments       : uint8_t  *buf		: transfer buffer pointer
                : uint32_t  size	: transfer data size
Return value    : none
******************************************************************************/
static void usb_cstd_dma_snd_setting(uint8_t *buf, uint32_t size)
{
volatile struct st_dmac1 __evenaccess *DMACA;
uint16_t block_size;
	if( g_usb_cstd_dma_fifo > size )
		block_size = ( size + 1 ) / 2;
	else
		block_size = ( g_usb_cstd_dma_fifo + 1 ) / 2;
	DMACA = R_DMACA_Address( );				// Set DMAC Channel Address
	DMACA->DMSTS.BIT.DTIF = 0;				// Clear Transfer Interrupt Flag
	(&ICU.DMRSR0)[R_DMACA_Channel( )<<2] = VECT( USB0, D0FIFO0 );	// Set DMRSR
    	DMACA->DMAMD.WORD = 0x8000;					// Set DMAMD
	DMACA->DMTMD.WORD = 0xA101;					// Set DMTMD
	DMACA->DMSAR = buf;						// Set DMSAR
	DMACA->DMDAR = fifo_address[USB_FIFO_ACCESS_TYPE_16BIT];	// Set DMSAR
	DMACA->DMCRA = block_size << 16 | block_size;			// Set DMCRA
	DMACA->DMCRB = ( size - 1 ) / g_usb_cstd_dma_fifo + 1;		// Set DMCRB
	DMACA->DMINT.BYTE = 0x10;					// Set DMINT
	R_DMACA_Int_Enable( );
	DMACA->DMCNT.BYTE = 0x01;				// Enable Transfer
}

/******************************************************************************
Function Name   : usb_cstd_dma_stop
Description     : DMA stop.
Arguments       : none
Return value    : none
******************************************************************************/
void usb_cstd_dma_stop(void)
{
volatile struct st_dmac1 __evenaccess *DMACA;
	DMACA = R_DMACA_Address( );			// Set DMAC Channel Address
	DMACA->DMSTS.BIT.DTIF = 0;			// Clear Transfer Interrupt Flag
}

/******************************************************************************
Function Name   : usb_cstd_dma_get_crtb
Description     : Get DMA Current Transaction Byte reg B(CRTB).
Arguments       : none
Return value    : DMA Current Transaction Byte reg B(CRTB)
******************************************************************************/
uint16_t usb_cstd_dma_get_crtb(void)
{
volatile struct st_dmac1 __evenaccess *DMACA;
	DMACA = R_DMACA_Address( );			// Set DMAC Channel Address
	return DMACA->DMCRB;				// Set Transfer Count
}
