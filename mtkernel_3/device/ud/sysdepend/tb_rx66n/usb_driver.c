/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usb_driver.c
 */

#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_ud.h>
#include "usb_config.h"
#include "iodefine.h"

LOCAL ID flgid;
LOCAL struct {
	UINT Exists : 1;
	UINT        :23;
	UINT USBType: 8;
	UINT BlockCount;
	UINT BlockSize;
} USBMem;

EXPORT INT USB_GetStatus(void)
{
	if( !USBMem.Exists || USBMem.USBType == UNKNOWN_MEM )	// USB Memory Exists ?
		return USB_NO_MEM;				// Non
	else
		return USB_RW_MEM;				// Read/Write OK
}

EXPORT UINT USB_GetBlockCount(void)
{
	if( USBMem.Exists == 1 && USBMem.USBType != UNKNOWN_MEM )
		return USBMem.BlockCount;
	return 0;
}

EXPORT ER USB_ReadBlock(void *buf, W start, SZ size)
{
	if( ! USBMem.Exists || USBMem.USBType == UNKNOWN_MEM )	// Check USB Type
		return E_NOMDA;
	if( buf == NULL || ((UW)buf & 0x3) )			// Check Parameter
		return E_PAR;
	if( usb_hmsc_strg_read_sector( buf, start, size, size * BLOCK_SIZE ) )
		return E_IO;					// Sector Read
	return USB_WaitRWEndEvent( );				// Wait Command End
}

EXPORT ER USB_WriteBlock(void *buf, W start, SZ size)
{
	if( ! USBMem.Exists || USBMem.USBType == UNKNOWN_MEM )	// Check USB Type
		return E_NOMDA;
	if( buf == NULL || ((UW)buf & 0x3) )			// Check Parameter
		return E_PAR;
	if( usb_hmsc_strg_write_sector( buf, start, size, size * BLOCK_SIZE ) )
		return E_IO;					// Sector Read
	return USB_WaitRWEndEvent( );				// Wait Command End
}

EXPORT void USB_Attach(void)
{
int i;
	USBMem.Exists = 1;					// USB Memory Attach
	for( i=0 ; i<500 ; i++ )  {
		usb_hmsc_strg_read_capacity( );			// Capacity Read
		if( USB_WaitRWEndEvent( ) == E_OK )  {		// Wait Command End
			USB_GetCapacity( &USBMem.BlockCount, &USBMem.BlockSize );
			if( USBMem.BlockSize == BLOCK_SIZE )  {
				USBMem.USBType = USB_MEM_STD;	// Set USB Type
				tm_printf("BlockCount = %d\n", USBMem.BlockCount);
				return ;
			}
		}
	}
	USB_NoSupport( );
}

EXPORT void USB_Detach(void)
{
	USBMem.Exists = 0;					// USB Memory Detach
}

EXPORT void USB_NoSupport(void)
{
	USBMem.USBType = UNKNOWN_MEM;				// Set USB Type
	tm_putstring("USB Memory is Unknow Type\n");
}

#if USB_CFG_DMA_CHANNEL >= 4
LOCAL void DMAC74I_Handler(UINT intno)
{
	if( DMAC.DMIST.BYTE & 0x10 )  {		// Channel 4 Interrupt ?
#if USB_CFG_DMA_CHANNEL == 4
		DMA_End_hdr( intno );
#endif
	}
	if( DMAC.DMIST.BYTE & 0x20 )  {		// Channel 5 Interrupt ?
#if USB_CFG_DMA_CHANNEL == 5
		DMA_End_hdr( intno );
#endif
	}
	if( DMAC.DMIST.BYTE & 0x40 )  {		// Channel 6 Interrupt ?
#if USB_CFG_DMA_CHANNEL == 6
		DMA_End_hdr( intno );
#endif
	}
	if( DMAC.DMIST.BYTE & 0x80 )  {		// Channel 7 Interrupt ?
#if USB_CFG_DMA_CHANNEL == 7
		DMA_End_hdr( intno );
#endif
	}
}
#endif

EXPORT ER USB_Init(ID objid, T_DINT *p_dint)
{
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( USB0 ) )  {					// USB0 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// USB0 is Already Enable
	}
	tk_ena_dsp( );						// Dispatch Enable

	p_dint->intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	p_dint->inthdr = USB_Int_hdr;				// Set Handler Address
#else
	p_dint->inthdr = (FP)USB_Int_hdr;			// Set Handler Address
#endif
	tk_def_int( USB_CFG_VECTOR_NUMBER, p_dint );		// Define Interrupt Handler
#if USB_CFG_DMA_CHANNEL < 4
#ifdef CLANGSPEC
	p_dint->inthdr = DMA_End_hdr;				// Set Handler Address
#else
	p_dint->inthdr = (FP)DMA_End_hdr;			// Set Handler Address
#endif
	tk_def_int( VECT( DMAC, DMAC0I ) + USB_CFG_DMA_CHANNEL, p_dint );
#else								// Define Interrupt Handler
#ifdef CLANGSPEC
	p_dint->inthdr = DMAC74I_Handler;			// Set Handler Address
#else
	p_dint->inthdr = (FP)DMAC74I_Handler;			// Set Handler Address
#endif
	tk_def_int( VECT( DMAC, DMAC74I ), p_dint );		// Define Interrupt Handler
#endif
								// Define Interrupt Handler
	flgid = objid;						// Set Interface EventFlag ID
	R_USB_Open( );						// Open USB
	return E_OK;						// Return
}

EXPORT void USB_InterruptEvent(void)
{
	tk_set_flg( flgid, USBEVENT );				// Set USB Interrupt Event
}

EXPORT void USB_AttachEvent(void)
{
	tk_set_flg( flgid, USBATTACH );				// Set USB Attach Event
}

EXPORT void USB_DetachEvent(void)
{
	tk_set_flg( flgid, USBDETACH );				// Set USB Detach Event
}

EXPORT void USB_NoSupportEvent(void)
{
	tk_set_flg( flgid, NOTSUPPORT );			// Set USB No Support Event
}

EXPORT void USB_StrgRWEndEvent(BOOL endcode)
{
	tk_set_flg( flgid, STRGRWEEND << endcode );		// Set Strage R/W End Event
}

EXPORT void USB_DmaCompleteEvent(void)
{
	tk_set_flg( flgid, DMACOMPLETE );			// Set DMA Complete Event
}

EXPORT ID USB_GetTaskPri(void)
{
	return USB_CFG_TASK_PRIORITY;				// Return Task Priority
}