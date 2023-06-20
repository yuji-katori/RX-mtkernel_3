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

#define	BSP_CFG_RTOS_USED			(0)
#define	R_BSP_VERSION_MAJOR			(5)
#define	R_BSP_VOLATILE_EVENACCESS		volatile __evenaccess
#define	R_BSP_EVENACCESS_SFR			__evenaccess
#define	R_BSP_PRAGMA(x)
#define	R_BSP_PRAGMA_PACK			R_BSP_PRAGMA(pack)
#define	R_BSP_PRAGMA_UNPACK			R_BSP_PRAGMA(unpack)
#define	R_BSP_PRAGMA_PACKOPTION			R_BSP_PRAGMA(packoption)
#define	R_BSP_PRAGMA_INTERRUPT(x,y)
#define	R_BSP_PRAGMA_STATIC_INTERRUPT(x,y)
#define	R_BSP_ATTRIB_INTERRUPT
#define	R_BSP_ATTRIB_STATIC_INTERRUPT
#define	R_BSP_RegisterProtectEnable(x)		RegisterProtectEnable( )
#define	R_BSP_RegisterProtectDisable(x)		RegisterProtectDisable( )
#define	VECT_USB0_USBI0				USB_CFG_VECTOR_NUMBER
#define	R_BSP_InterruptRequestEnable(x)		InterruptRequestEnable(x)
#define	R_BSP_InterruptRequestDisable(x)	InterruptRequestDisable(x)
void RegisterProtectEnable(void);
void RegisterProtectDisable(void);
void InterruptRequestEnable(unsigned int intno);
void InterruptRequestDisable(unsigned int intno);
void usbfs_usbi_isr(void);
#ifdef USB_DEBUG_ON
int  tm_printf( const char *format, ... );
#endif /* USB_DEBUG_ON */
#define	IPR_USB0_USBI0				(USB_CFG_VECTOR_NUMBER)
#define	bool					_Bool
#define	false					(0)
#define	true					(1)
#define R_BSP_ATTRIB_STRUCT_BIT_ORDER_RIGHT(bf0, bf1, bf2, bf3, bf4, bf5, bf6, bf7, bf8, bf9, \
                                            bf10, bf11, bf12, bf13, bf14, bf15, bf16, bf17, bf18, bf19, \
                                            bf20, bf21, bf22, bf23, bf24, bf25, bf26, bf27, bf28, bf29, \
                                            bf30, bf31)\
struct {\
R_BSP_PRAGMA(bit_order right)\
    struct {\
        bf0;\
        bf1;\
        bf2;\
        bf3;\
        bf4;\
        bf5;\
        bf6;\
        bf7;\
        bf8;\
        bf9;\
        bf10;\
        bf11;\
        bf12;\
        bf13;\
        bf14;\
        bf15;\
        bf16;\
        bf17;\
        bf18;\
        bf19;\
        bf20;\
        bf21;\
        bf22;\
        bf23;\
        bf24;\
        bf25;\
        bf26;\
        bf27;\
        bf28;\
        bf29;\
        bf30;\
        bf31;\
    };\
R_BSP_PRAGMA(bit_order)\
}
#define R_BSP_ATTRIB_STRUCT_BIT_ORDER_RIGHT_2(bf0, bf1)\
 R_BSP_ATTRIB_STRUCT_BIT_ORDER_RIGHT( \
 bf0, bf1, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, \
 uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, \
 uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, \
 uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0, uint8_t :0) \

#define USB_PATH_(a)			#a
#define USB_PATH(a)			USB_PATH_(a)
#define USB_BSP()			USB_PATH(../sysdepend/TARGET_DIR/usb_config.h)
#include USB_BSP()

#endif	/* PLATFORM_H */
