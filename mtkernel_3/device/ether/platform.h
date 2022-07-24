/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	platform.h
 */

/* Multiple inclusion prevention macro */
#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include "iodefine.h"

#ifndef	ETHERC0
#define	ETHERC0		ETHERC
#endif
#ifndef	EDMAC0
#define	EDMAC0		EDMAC
#define	MSTP_EDMAC0	MSTP_EDMAC
#endif

#define	__ITRON_DATA_TYPE
#define	R_BSP_VERSION_MAJOR		(5)
#define	BSP_CFG_RTOS_USED		(2)
#define	APPLICATION_T4_BLOCKING_TYPE	(0)

#define	R_BSP_HardwareLock(x)		true
#define	R_BSP_HardwareUnlock(x)		true
#define	R_BSP_RegisterProtectDisable(x)	(SYSTEM.PRCR.WORD = 0xA502)
#define	R_BSP_RegisterProtectEnable(x)	(SYSTEM.PRCR.WORD = 0xA500)
#define	R_BSP_InterruptsEnable(x)

#define	BSP_CFG_USER_LOCKING_TYPE	uint16_t

typedef void (*bsp_int_cb_t)(void *);

typedef enum {
	BSP_INT_SUCCESS = 0,
	BSP_INT_ERR_NO_REGISTERED_CALLBACK,
	BSP_INT_ERR_INVALID_ARG,
	BSP_INT_ERR_UNSUPPORTED,
	BSP_INT_ERR_GROUP_STILL_ENABLED,
	BSP_INT_ERR_INVALID_IPL
} bsp_int_err_t;

typedef enum {
    BSP_INT_SRC_AL1_EDMAC0_EINT0 = 0,
    BSP_INT_SRC_AL1_EDMAC1_EINT1,
} bsp_int_src_t;

#define	bool			_Bool
#ifndef	false
#define	false			(0)
#endif
#ifndef	true
#define	true			(1)
#endif

#ifndef	__TK_TYPEDEF_H__
typedef signed char	B;
typedef signed short	H;
typedef signed long	W;
typedef unsigned char	UB;
typedef unsigned short	UH;
typedef unsigned long	UW;
typedef signed int	INT;
typedef void *		VP;
typedef INT		ID;
typedef UW		ATR;
typedef INT		ER;
typedef W		TMO;
#endif

#define	FIT_NO_PTR		NULL
#define	FIT_NO_FUNC		NULL

#define	R_BSP_NOP()		__nop( )
#define	R_BSP_PRAGMA(...)	_Pragma(#__VA_ARGS__)
#define	R_BSP_PRAGMA_PACK	R_BSP_PRAGMA(pack)
#define	R_BSP_PRAGMA_UNPACK	R_BSP_PRAGMA(unpack)
#define	R_BSP_PRAGMA_PACKOPTION	R_BSP_PRAGMA(packoption)
#define	R_BSP_EVENACCESS	__evenaccess
#define	R_BSP_EVENACCESS_SFR	__evenaccess
#define	R_BSP_VOLATILE_EVENACCESS				volatile __evenaccess
#define	__R_BSP_ATTRIB_SECTION_CHANGE_V(type, section_name)	R_BSP_PRAGMA(section type section_name)
#define	_R_BSP_ATTRIB_SECTION_CHANGE_B1(section_tag)		__R_BSP_ATTRIB_SECTION_CHANGE_V(B, B##section_tag)
#define	R_BSP_ATTRIB_SECTION_CHANGE(type, section_tag, ...)	_R_BSP_ATTRIB_SECTION_CHANGE_##type##__VA_ARGS__(section_tag)
#define	R_BSP_ATTRIB_SECTION_CHANGE_END				R_BSP_PRAGMA(section)

bsp_int_err_t R_BSP_InterruptWrite(bsp_int_src_t vector,  bsp_int_cb_t callback);
void ether_tsk(INT stacd, void *exinf);
void ether_cychdr(void *pdata);
void Initialize_Ether(void);

#endif /* PLATFORM_H */