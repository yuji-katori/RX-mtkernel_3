/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dev_iic.h
 */
#ifndef __DEV_QSPI_H__
#define __DEV_QSPI_H__

#define	QSPISF_DEVNM		"QSPISF"
// EventFlag Bit Pattarn
#define QSPI_WAITALL		0x00000003
#define QSPI_EXECCMD		0x00000002
#define QSPI_WRITEIP		0x00000001
#define QSPI_MINIMUM		0x00000020
#define QSPI_MAXIMUM		0x80000000

typedef enum {
	DN_WTADDR = -100,	// (W) Set Write Address
	DN_RDADDR = -101,	// (W) Set Read Address
	DN_RDSR   = -102,	// (R) Read Status Register
	DN_RDCR   = -103,	// (R) Read Configuration Register
	DN_WRSR   = -104,	// (W) Write Status Register
	DN_SE     = -105,	// (W) Section Erase (4K Byte)
	DN_BE32K  = -106,	// (W) Block Erase (32K Byte)
	DN_BE     = -107,	// (W) Block Erase (64K Byte)
	DN_CE     = -108,	// (W) Chip Erase
	DN_PP     =    0,	// (W) Page Program
	DN_FREAD  =    1,	// (R) Frst Read
	DN_DREAD  =    2,	// (R) Dual Read
	DN_QREAD  =    3,	// (R) Quad Read
} SeriFlashComm;

IMPORT void drv_lock(INT mode, W *lock);
IMPORT void QSPI_Init(void);
IMPORT void QSPI_Read(T_DEVREQ *devreq);
IMPORT TMO  QSPI_Write(T_DEVREQ *devreq);
IMPORT TMO  QSPI_WtCheck(void);
IMPORT PRI  QSPI_GetTaskPri(void);
IMPORT ER   qspiDrvEntry(void);

#endif /* __DEV_IIC_H__ */