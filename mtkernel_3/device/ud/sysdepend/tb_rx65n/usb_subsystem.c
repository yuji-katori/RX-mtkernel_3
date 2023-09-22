/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usb_subsystem.c
 */

#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_ud.h>
#include "usb_config.h"
#include "iodefine.h"

EXPORT void RegisterProtectEnable(void)
{
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	tk_ena_dsp( );						// Dispatch Enable
}

EXPORT void RegisterProtectDisable(void)
{
	tk_dis_dsp( );						// Dispatch Disable
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
}

EXPORT void InterruptRequestEnable(UINT intno)
{
	if( intno >= 128 )
		(&ICU.SLIBXR128.BYTE)[intno-128] = 62;		// Set Interrupt Factor
	ICU.IER[intno>>3].BYTE |= 1<<(intno&7);			// Enable Interrupt
}

EXPORT void InterruptRequestDisable(UINT intno)
{
	ICU.IER[intno>>3].BYTE &= ~(1<<(intno&7));		// Disable Interrupt
	if( intno >= 128 )
		(&ICU.SLIBXR128.BYTE)[intno-128] = 0;		// Clear Interrupt Factor
}

EXPORT BOOL R_USB_VBUS(void)
{
	return 0;
}

EXPORT void R_DMACA_Open(void)
{
	tk_dis_dsp( );						// Dispatch Disable
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( DMAC ) = 0;					// Enable DMAC
	tk_ena_dsp( );						// Dispatch Enable
	DMAC.DMAST.BYTE = 0x01;					// Enable DMAC Activation
}

EXPORT void R_DMACA_Close(void)
{
	(&DMAC0.DMCNT.BYTE)[USB_CFG_DMA_CHANNEL<<2] = 0x00;	// Disable DMA Transfers
	(&ICU.DMRSR0)[USB_CFG_DMA_CHANNEL<<2] = 0x00;		// Clear DMRSRn
}

EXPORT void R_DMACA_Int_Enable(void)
{
UB vecnum;

#if USB_CFG_DMA_CHANNEL < 4
	vecnum = VECT( DMAC, DMAC0I ) + USB_CFG_DMA_CHANNEL;	// DMAC0 ` DMAC3 Vector Number
	ICU.IPR[vecnum].BYTE = USB_CFG_INT_PRIORITY;		// Set Interrpt Priority Level
#else
	vecnum = VECT( DMAC, DMAC74I );				// DMAC4 ` DMAC7 Vector Number
	if( IPR( DMAC, DMAC74I ) < USB_CFG_INT_PRIORITY )	// Interrupt Level Check
		IPR( DMAC, DMAC74I ) = USB_CFG_INT_PRIORITY;	// Set Interrupt Priority Level
#endif
	ICU.IER[vecnum>>3].BYTE |= 1<<(vecnum&7);		// Enable Interrupt
}

EXPORT void R_DMACA_Int_Disable(void)
{
UB vecnum;

#if USB_CFG_DMA_CHANNEL < 4
	vecnum = IPR( DMAC, DMAC0I ) + USB_CFG_DMA_CHANNEL;	// DMAC0 ` DMAC3 Vector Number
#else
	vecnum = IPR( DMAC, DMAC74I );				// DMAC4 ` DMAC7 Vector Number
#endif
	ICU.IPR[vecnum].BYTE = 0;				// Clear Interrpt Priority Level
	ICU.IER[vecnum>>3].BYTE &= ~(1<<(vecnum&7));		// Disable Interrupt
}

EXPORT volatile struct st_dmac1 __evenaccess *R_DMACA_Address(void)
{
volatile struct st_dmac1 __evenaccess *DMACA = (volatile struct st_dmac1 __evenaccess *)0x82000;
	DMACA += USB_CFG_DMA_CHANNEL << 1;			// Adjust DMAC Channel Address
	return DMACA;
}

EXPORT UB R_DMACA_Channel(void)
{
	return USB_CFG_DMA_CHANNEL;
}