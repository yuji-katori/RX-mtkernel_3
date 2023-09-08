/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	platform.h
 */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stddef.h>
#include "sys/machine.h"
#include "iodefine.h"

#ifndef	VECT_USB0_USBI0
#define	VECT_USB0_USBI0				USB_CFG_VECTOR_NUMBER
#define	IPR_USB0_USBI0				(USB_CFG_VECTOR_NUMBER)
#endif	/* VECT_USB0_USBI0 */
#define	R_BSP_InterruptRequestEnable(x)		InterruptRequestEnable(x)
#define	R_BSP_InterruptRequestDisable(x)	InterruptRequestDisable(x)
void RegisterProtectEnable(void);
void RegisterProtectDisable(void);
void InterruptRequestEnable(unsigned int intno);
void InterruptRequestDisable(unsigned int intno);
#ifdef USB_DEBUG_ON
int  tm_printf( const char *format, ... );
#endif /* USB_DEBUG_ON */
#define	bool					_Bool

#define USB_PATH_(a)			#a
#define USB_PATH(a)			USB_PATH_(a)
#define USB_BSP()			USB_PATH(../sysdepend/TARGET_DIR/usb_config.h)
#include USB_BSP()

#endif	/* PLATFORM_H */
