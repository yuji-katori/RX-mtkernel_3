/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hreg_access.c
 */

#include "r_usb_typedef.h"
#include "r_usb_bitdefine.h"
#include "r_usb_extern.h"

/******************************************************************************
 Function Name   : hw_usb_hmodule_init
 Description     : USB module initialization for USB Host mode.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void hw_usb_hmodule_init(void)
{
uint16_t sts;
	USB0.SYSCFG.WORD |= USB_SCKE;
	while( USB_SCKE != (USB0.SYSCFG.WORD & USB_SCKE) )
		__nop( );
#if defined(BSP_MCU_RX64M)
	USB0.PHYSLEW.LONG = 0xE;
#endif	/* defined(BSP_MCU_RX64M) */
#if defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72T) || defined (BSP_MCU_RX72M)\
 || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671)
	USB0.PHYSLEW.LONG = 0x5;
#endif  /* defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72T) || defined (BSP_MCU_RX72M)
 || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
	USB0.SYSCFG.WORD |= USB_DCFM;
	USB0.SYSCFG.WORD |= USB_DRPD;
	sts = usb_chattaring( &USB0.SYSSTS0.WORD );
	USB0.SYSCFG.WORD |= USB_USBE;
	USB0.CFIFOSEL.WORD  = USB0_CFIFO_MBW;
	USB0.D0FIFOSEL.WORD = USB0_D0FIFO_MBW;
	USB0.D1FIFOSEL.WORD = USB0_D1FIFO_MBW;
#if USB_CFG_ENDIAN == USB_CFG_BIG
	USB0.CFIFOSEL.WORD  |= USB_BIGEND;
	USB0.D0FIFOSEL.WORD |= USB_BIGEND;
	USB0.D1FIFOSEL.WORD |= USB_BIGEND;
#endif	/* USB_CFG_ENDIAN == USB_CFG_BIG */
	switch( sts )  {
	case USB_FS_JSTS:			 // USB device already connected
	case USB_LS_JSTS:
		USB0.DVSTCTR0.WORD |= USB_USBRST;
		usb_cpu_delay_xms( 50 );	// Need to wait greater equal 10ms in USB spec
		USB0.DVSTCTR0.WORD &= ~USB_USBRST;
		while( USB_HSPROC == ( USB0.DVSTCTR0.WORD & USB_HSPROC ) )
			__nop( );
		if( USB_LSMODE == ( USB0.DVSTCTR0.WORD & USB_RHST ) )
			USB0.SOFCFG.WORD |= USB_TRNENSEL;
		USB0.DVSTCTR0.WORD |= USB_UACT;
		break;
	case USB_SE0:				// USB device no connected
		USB0.INTENB1.WORD = USB_ATTCH;
		break;
        }
	USB0.INTSTS1.WORD &= ~USB_OVRCRE & INTSTS1_MASK;
	USB0.INTENB0.WORD = USB_BEMPE | USB_NRDYE | USB_BRDYE;
	USB0.INTENB1.WORD = USB_OVRCRE | USB_ATTCH;
}
