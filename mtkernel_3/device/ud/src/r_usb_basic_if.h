/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_basic_if.h
 */

#ifndef R_USB_BASIC_IF_H
#define R_USB_BASIC_IF_H

#include "platform.h"
#include "r_usb_basic_define.h"
#include "r_usb_basic_config.h"

// USB Standard request
#define	USB_CLEAR_FEATURE			(0x0100)
#define	USB_SET_FEATURE				(0x0300)
#define	USB_SET_ADDRESS				(0x0500)
#define	USB_GET_DESCRIPTOR			(0x0600)
#define	USB_SET_CONFIGURATION			(0x0900)

// USB_BMREQUESTTYPEDIR 0x0080u(b7)
#define	USB_HOST_TO_DEV				(0x0000)
#define	USB_DEV_TO_HOST				(0x0080)

// USB_BMREQUESTTYPETYPE    0x0060u(b6-5)
#define	USB_STANDARD				(0x0000)

// USB_BMREQUESTTYPERECIP   0x001Fu(b4-0)
#define	USB_DEVICE				(0x0000)
#define	USB_ENDPOINT				(0x0002)

#define	USB_NULL				(0)

// USB pipe number
#define	USB_PIPE0				(0)
#define	USB_PIPE1				(1)

// Descriptor type  Define
#define	USB_DT_CONFIGURATION			(0x02)		// Configuration Descriptor
#define	USB_DT_INTERFACE			(0x04)		// Interface Descriptor
#define	USB_DT_ENDPOINT				(0x05)		// Endpoint Descriptor

// Interface class Define
#define	USB_IFCLS_NOT				(0x00)		// Un corresponding Class
#define	USB_IFCLS_MAS				(0x08)		// Mass Storage Class

// Endpoint Descriptor  Define
#define	USB_EP_IN				(0x80)		// In Endpoint
#define	USB_EP_BULK				(0x02)		// Bulk Transfer
#define	USB_EP_INT				(0x03)		// Interrupt Transfer

// Configuration descriptor bit define
#define	USB_CF_RWUPON				(0x20)		// Remote Wake up ON

typedef struct usb_setup {
	uint16_t	type;		// USB standard/class request type
	uint16_t	value;		// Request value
	uint16_t	index;		// Request index
	uint16_t	length;		// Request length
} usb_setup_t;

// USB class type
typedef enum usb_class {
	USB_PCDC = 0, USB_PCDCC, USB_PCDC2, USB_PCDCC2, USB_PHID, USB_PVND,
	USB_PCDC_PHID, USB_PCDC_PMSC, USB_PHID_PMSC,
	USB_HCDC, USB_HCDCC, USB_HHID, USB_HVND,
	USB_HMSC, USB_PMSC, USB_REQUEST
} usb_class_t;

// USB status
typedef enum usb_onoff {
	USB_OFF = 0, USB_ON,
} usb_onoff_t;

void R_USB_Open(void);
void R_USB_Close(void);
bool R_USB_VBUS(void);
void R_DMACA_Open(void);
void R_DMACA_Close(void);
void R_DMACA_Int_Enable(void);
void R_DMACA_Int_Disable(void);
volatile struct st_dmac1 __evenaccess *R_DMACA_Address(void);
uint8_t R_DMACA_Channel(void);

#endif	/* R_USB_BASIC_IF_H */
