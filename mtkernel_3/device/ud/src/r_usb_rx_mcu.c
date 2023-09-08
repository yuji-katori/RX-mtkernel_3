/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_rx_mcu.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_bitdefine.h"
#include "tk/tkernel.h"

/******************************************************************************
 Function Name   : usb_module_start
 Description     : USB module start.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_module_start(void)
{
#if defined (BSP_MCU_RX72T)
	R_BSP_VoltageLevelSetting( BSP_VOL_USB_POWERON );
#endif	/* defined (BSP_MCU_RX72T) */
	RegisterProtectDisable( );		// Enable writing to MSTP registers
        MSTP( USB0 ) = 0;			// Enable power for USB0
        RegisterProtectEnable( );		// Disable writing to MSTP registers
}

/******************************************************************************
 Function Name   : usb_module_stop
 Description     : USB module stop.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_module_stop(void)
{
	USB0.DVSTCTR0.WORD = 0;
	USB0.DCPCTR.WORD = USB_SQSET;
	USB0.PIPE1CTR.WORD = 0;
	USB0.PIPE2CTR.WORD = 0;
	USB0.PIPE3CTR.WORD = 0;
	USB0.PIPE4CTR.WORD = 0;
	USB0.PIPE5CTR.WORD = 0;
	USB0.PIPE6CTR.WORD = 0;
	USB0.PIPE7CTR.WORD = 0;
	USB0.PIPE8CTR.WORD = 0;
	USB0.PIPE9CTR.WORD = 0;
	USB0.BRDYENB.WORD = 0;
	USB0.NRDYENB.WORD = 0;
	USB0.BEMPENB.WORD = 0;
	USB0.INTENB0.WORD = 0;
	USB0.INTENB1.WORD = 0;
	USB0.SYSCFG.WORD &= (~USB_DPRPU);
	USB0.SYSCFG.WORD &= (~USB_DRPD);
	USB0.SYSCFG.WORD &= (~USB_USBE);
	USB0.SYSCFG.WORD &= (~USB_DCFM);
	USB0.BRDYSTS.WORD = 0;
	USB0.NRDYSTS.WORD = 0;
	USB0.BEMPSTS.WORD = 0;
	USB0.DEVADD0.WORD = 0;
	USB0.DEVADD1.WORD = 0;
	USB0.DEVADD2.WORD = 0;
	USB0.DEVADD3.WORD = 0;
	USB0.DEVADD4.WORD = 0;
	USB0.DEVADD5.WORD = 0;
#if defined (BSP_MCU_RX72T)
	R_BSP_VoltageLevelSetting( BSP_VOL_USB_POWEROFF );
#endif	/* defined (BSP_MCU_RX72T) */
	RegisterProtectDisable( );		// Enable writing to MSTP registers
	MSTP( USB0 ) = 1;			// Disable power for USB0
        RegisterProtectEnable( );		// Disable writing to MSTP registers
}

/******************************************************************************
 Function Name   : usb_cpu_usbint_init
 Description     : USB interrupt Initialize.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_cpu_usbint_init(void)
{
#if (!defined (BSP_MCU_RX63T)) && (!defined (BSP_MCU_RX72T))
	/* Deep standby USB monitor register
	b0      SRPC0    USB0 single end control
	b3-b1   Reserved 0
	b4      FIXPHY0  USB0 transceiver output fix
	b7-b5   Reserved 0
	b8      SRPC1    USB1 single end control
	b11-b9  Reserved 0
	b12     FIXPHY1  USB1 transceiver output fix
	b15-b13 Reserved 0
	b16     DP0      USB0 DP input
	b17     DM0      USB0 DM input
	b19-b18 Reserved 0
	b20     DOVCA0   USB0 OVRCURA input
	b21     DOVCB0   USB0 OVRCURB input
	b22     Reserved 0
	b23     DVBSTS0  USB1 VBUS input
	b24     DP1      USB1 DP input
	b25     DM1      USB1 DM input
	b27-b26 Reserved 0
	b28     DOVCA1   USB1 OVRCURA input
	b29     DOVCB1   USB1 OVRCURB input
	b30     Reserved 0
	b31     DVBSTS1  USB1 VBUS input
	*/
	USB.DPUSR0R.BIT.FIXPHY0 = 0;	// USB0 Transceiver Output fixed
#endif /* (!defined (BSP_MCU_RX63T)) && (!defined (BSP_MCU_RX72T)) */
	/* Interrupt enable register
	b0 IEN0 Interrupt enable bit
	b1 IEN1 Interrupt enable bit
	b2 IEN2 Interrupt enable bit
	b3 IEN3 Interrupt enable bit
	b4 IEN4 Interrupt enable bit
	b5 IEN5 Interrupt enable bit
	b6 IEN6 Interrupt enable bit
	b7 IEN7 Interrupt enable bit
	*/
	R_BSP_InterruptRequestEnable( VECT( USB0, D0FIFO0 ) );	// D0FIFO0 Enable

#if defined (BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671)
	R_BSP_InterruptRequestEnable( VECT( USB0, USBR0 ) );	// USBR0 enable
#endif  /* defined (BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
#if defined (BSP_MCU_RX63N) || defined (BSP_MCU_RX62N)
	R_BSP_InterruptRequestEnable( VECT( USB, USBR0 ) );	// USBR0 enable
#endif  /* defined (BSP_MCU_RX63N) || defined (BSP_MCU_RX62N) */
	/* Interrupt priority register
	b3-b0 IPR      Interrupt priority
	b7-b4 Reserved 0
	*/
#if defined (BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671)
	IPR( USB0, USBR0 ) = 0x00;				// USBR0
#endif  /* defined (BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX72T)\
    || defined (BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
#if defined (BSP_MCU_RX63N) || defined (BSP_MCU_RX62N)
	IPR( USB, USBR0 ) = 0x00;				// USBR0
#endif  /* defined (BSP_MCU_RX63N) || defined (BSP_MCU_RX62N) */
	IPR( USB0, USBI0 ) = USB_CFG_INTERRUPT_PRIORITY;	// USBI0 in vector 128
	R_BSP_InterruptRequestEnable( VECT( USB0, USBI0 ) );	// USBI0 enable in vector 128
}

/******************************************************************************
 Function Name   : usb_cpu_delay_1us
 Description     : 1us Delay timer.
 Arguments       : uint16_t time	: Delay time(*1us)
 Return value    : none
 ******************************************************************************/
void usb_cpu_delay_1us(uint16_t time)
{
int i;
	for( i=ICLK/4*time ; i  ; i-- )  ;			// ICLK/4cyc*time
}

/******************************************************************************
 Function Name   : usb_cpu_delay_xms
 Description     : xms Delay timer
 Arguments       : uint16_t time	: Delay time(*1ms)
 Return value    : void
 ******************************************************************************/
void usb_cpu_delay_xms(uint16_t time)
{
	tk_dly_tsk( time );
}

/******************************************************************************
 Function Name   : usb_chattaring
 Description     : Remove chattaring processing.
 Arguments       : uint16_t *syssts	: SYSSTS register value
 Return value    : LNST bit value
 ******************************************************************************/
uint16_t usb_chattaring(volatile __evenaccess uint16_t *syssts)
{
uint16_t lnst[4];

	while( 1 )  {
		lnst[0] = (*syssts) & USB_LNST;
		usb_cpu_delay_xms( 1 );			// Wait 1ms
		lnst[1] = (*syssts) & USB_LNST;
		usb_cpu_delay_xms( 1 );			// Wait 1ms
		lnst[2] = (*syssts) & USB_LNST;
		if( lnst[0] == lnst[1] && lnst[0] == lnst[2] )
			break;
	}
	return lnst[0];
}

/*******************************************************************************
 * Function Name: USB_Int_hdr
 * Description  : Interrupt service routine of USBF.
 * Arguments    : none
 * Return Value : none
 *******************************************************************************/
void USB_Int_hdr(void)
{
	usb_hstd_usb_handler( );		// Call interrupt routine
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M)
	ICU.PIBR7.BYTE |= 0x40;			// Flag clear
#endif	/* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX71M) */
}
