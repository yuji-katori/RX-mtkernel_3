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
#include "r_usb_basic_if.h"
#include "r_usb_hmsc_if.h"
#include "r_usb_hmsc.h"
#include "r_usb_dmac.h"
#include "iodefine.h"

LOCAL ID flgid;
LOCAL usb_ctrl_t ctrl;
LOCAL usb_cfg_t  cfg;
LOCAL usb_utr_t	 utr;
LOCAL UB side, buf[64];
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
	if( usb_hmsc_strg_read_sector( &utr, side, buf, start, size, size * BLOCK_SIZE ) )
		return E_IO;					// Sector Read
	if( usb_hmsc_strg_wait_cmdend( side ) )			// Wait Command End
		return E_IO;
	return E_OK;
}

EXPORT ER USB_WriteBlock(void *buf, W start, SZ size)
{
	if( ! USBMem.Exists || USBMem.USBType == UNKNOWN_MEM )	// Check USB Type
		return E_NOMDA;
	if( buf == NULL || ((UW)buf & 0x3) )			// Check Parameter
		return E_PAR;
	if( usb_hmsc_strg_write_sector( &utr, side, buf, start, size, size * BLOCK_SIZE ) )
		return E_IO;					// Sector Read
	if( usb_hmsc_strg_wait_cmdend( side ) )			// Wait Command End
		return E_IO;
	return E_OK;
}

EXPORT void USB_GetCapacity(void)
{
	R_USB_HmscStrgCmd( &ctrl, buf, USB_ATAPI_READ_CAPACITY);// Get USB Capacity
	R_USB_HmscGetDriveNo( &ctrl, &side );			// Get Driver Number
	usb_hmsc_smp_drive2_addr( side, &utr );			// Get Pointer of USB Structure
}

EXPORT UINT USB_Schedule(void)
{
	switch( R_USB_GetEvent( &ctrl ) )  {
	case USB_STS_CONFIGURED:
		USBMem.Exists = 1;				// USB Memory Attach
		USBMem.USBType = UNKNOWN_MEM;			// Set USB Type
		tk_set_flg( flgid,  USBATTACH );		// Set USB Attach Flag
		break;
	case USB_STS_DETACH:
		USBMem.Exists = 0;				// USB Memory Detach
		tk_set_flg( flgid,  USBDETACH );		// Set USB Detach Flag
		break;
	case USB_STS_MSC_CMD_COMPLETE:
		USBMem.BlockCount = (buf[0]<<24) + (buf[1]<<16) + (buf[2]<<8) + buf[3];
		USBMem.BlockSize  = (buf[4]<<24) + (buf[5]<<16) + (buf[6]<<8) + buf[7];
		if( USBMem.BlockSize == BLOCK_SIZE )  {
			USBMem.USBType = USB_MEM_STD;		// Set USB Type
			tm_printf("BlockCount = %d\n", USBMem.BlockCount);
		}
		else
			tm_putstring("Attach USB Memory is Unknow Type\n");
		tk_set_flg( flgid,  USBCAPACITY );		// Set USB Get Capacity Flag
	}
	return E_OK;						// Get USB Event Code
}

EXPORT ER USB_Init(ID objid, T_DINT *p_dint)
{
ER ercd;
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( USB0 ) )  {					// USB0 is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// USB0 is Already Enable
	}
	tk_ena_dsp( );						// Dispatch Enable

	PORTC.PDR.BIT.B3 = 1;					// USB Host Setting
	PORTC.PODR.BIT.B3 = 1;					// Enable USB Select
	PORTC.PDR.BIT.B1 = 1;					// VBUS Setting
	PORTC.PODR.BIT.B1 = 1;					// Enable VBUS Select

	p_dint->intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	p_dint->inthdr = (INTFP)usbfs_usbi_isr;			// Set Handler Address
#else
	p_dint->inthdr = (FP)usbfs_usbi_isr;			// Set Handler Address
#endif
	tk_def_int( USB_CFG_VECTOR_NUMBER, p_dint );		// Define Interrupt Handler
#if USB_CFG_DMA == USB_CFG_ENABLE
#ifdef CLANGSPEC
	p_dint->inthdr = (INTFP)usb_cstd_ip0_d0fifo_cb;		// Set Handler Address
#else
	p_dint->inthdr = (FP)usb_cstd_ip0_d0fifo_cb;		// Set Handler Address
#endif
	tk_def_int( VECT( DMAC, DMAC0I ) + USB_CFG_TX_DMA_CHANNEL, p_dint );
#endif	/* USB_CFG_DMA == USB_CFG_ENABLE */
								// Define Interrupt Handler
	flgid = objid;						// Set Interface EventFlag ID
	ctrl.module     = USB_IP0;				// USB Channel is USB0
	ctrl.type       = USB_HMSC;				// USB Type is HMSC
	cfg.usb_speed   = USB_FS;				// USB Full Speed
	cfg.usb_mode    = USB_HOST;				// USB Host
	ercd = - R_USB_Open( &ctrl, &cfg );			// Open USB
	return ercd;						// Return
}

EXPORT void USB_SetEvent(void)
{
	tk_set_flg( flgid, USBEVENT );				// Set USB Event
}

EXPORT void USB_ClearEvent(BOOL flg)
{
	if( flg )
		tk_set_flg( flgid, USBEVENT );			// Set USB Event
	else
		tk_clr_flg( flgid, ~USBEVENT );			// Clear USB Event
}

EXPORT void USB_WaiEvent(void)
{
UINT flgptn;							// Wait USB Event
	tk_wai_flg( flgid, USBEVENT, TWF_ORW | TWF_BITCLR, &flgptn, 10 );
}

EXPORT ID USB_GetTaskPri(void)
{
	return USB_CFG_TASK_PRIORITY;				// Return Task Priority
}