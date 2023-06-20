/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	dev_sd.h
 */
#ifndef __DEV_SD_H__
#define __DEV_SD_H__

#include <dev_disk.h>

// SD Card Status
#define SD_NO_CARD		0
#define SD_RO_CARD		1
#define SD_RW_CARD		2

// SD Command Index
#define GO_IDLE_STATE		0
#define ALL_SEND_CID		2
#define SET_REL_ADDR		3
#define SEL_DESEL_CARD		7
#define SEND_IF_COND		8
#define SEND_CSD		9
#define SEND_STATUS		13
#define SET_BLOCKLEN		16
#define READ_SINGLE_BLOCK	17
#define READ_MULT_BLOCK		18
#define WRITE_SINGLE_BLOCK	24
#define WRITE_MULT_BLOCK	25
#define APP_CMD			55
#define SET_BUS_WIDTH	       ( 6+0x40)
#define SD_APP_OP_COND	       (41+0x40)

// SD Memory Cards Kind
#define UNKNOWN_CARD		0
#define SD_CARD_STD		1
#define SD_CARD_HIGH		2

// Mask for Errors Card Status R1 (OCR Register)
#define OCR_ERRORBITS		0xFDFFE008
// Masks for R6 Response
#define R6_GENERAL_UNKNOWN	0x00002000
#define R6_ILLEGAL_CMD		0x00004000
#define R6_COM_CRC_FAILED	0x00008000

#define WAIT_TIME		1000
#define MAX_TRIAL		100
#define	BLOCK_SIZE		512

// EventFlag Bit Pattarn
#define EXECCMD			0x00000100
#define CARD_REJECT		0x00000400
#define CARD_INSERT		0x00000800
#define CMD_COMPLETE		0x00000080
#define TRANS_COMPLETE		0x00000200
#define CMD_TIMEOUT		0x00000040
#define SDHC_ERROR		0x0000003F
#define TSK_WAIT_ALL		0x00000D00
#define REJECT			0x00001000
#define INSERT			0x00002000
#define MINIMUM			0x00002000
#define MAXIMUM			0x80000000

IMPORT INT  SDC_GetStatus(void);
IMPORT UINT SDC_GetBlockCount(void);
IMPORT ER   SDC_ReadBlock(void *buf, W start, SZ size);
IMPORT ER   SDC_WriteBlock(void *buf, W start, SZ size);
IMPORT ER   SDC_CardInsert(void);
IMPORT ER   SDC_InitCard(void);
IMPORT void SDC_CardReject(void);
IMPORT ER   SDC_Init(ID flgid,T_DINT *p_dint);
IMPORT PRI  SDC_GetTaskPri(void);

#endif /* __DEV_SD_H__ */