/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2023/9/8.
 *    Modified by Yuji Katori at 2023/9/11.
 *----------------------------------------------------------------------
 */

/*
 *	sd_driver.c
 */

#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_sd.h>
#include "sd_config.h"
#include "iodefine.h"

#if SD_CFG_DMA_CHANNEL == 0
#define SD_DMAC	DMAC0
#elif SD_CFG_DMA_CHANNEL == 1
#define SD_DMAC	DMAC1
#elif SD_CFG_DMA_CHANNEL == 2
#define SD_DMAC	DMAC2
#elif SD_CFG_DMA_CHANNEL == 3
#define SD_DMAC	DMAC3
#elif SD_CFG_DMA_CHANNEL == 4
#define SD_DMAC	DMAC4
#elif SD_CFG_DMA_CHANNEL == 5
#define SD_DMAC	DMAC5
#elif SD_CFG_DMA_CHANNEL == 6
#define SD_DMAC	DMAC6
#else
#define SD_DMAC	DMAC7
#endif

LOCAL ID flgid;
LOCAL struct {
	UINT Exists  : 1;
	UINT Protect : 1;
	UINT         :22;
	UINT CardType: 8;
	UINT RCA;
	UINT BlockCount;
	UINT BlockSize;
} SDCard;

EXPORT INT SDC_GetStatus(void)
{
	if( !SDCard.Exists || SDCard.CardType == UNKNOWN_CARD )	// SD Card Exists ?
		return SD_NO_CARD;				// Non
	else if( SDCard.Protect  )				// Write Protect ?
		return SD_RO_CARD;				// Read Only
	else
		return SD_RW_CARD;				// Read/Write OK
}

EXPORT UINT SDC_GetBlockCount(void)
{
	if( SDCard.Exists == 1 && SDCard.CardType != UNKNOWN_CARD )
		return SDCard.BlockCount;
	return 0;
}

LOCAL ER SDC_SndCmd(UINT cmd, UINT arg)
{
UINT flgptn;
ER ercd;

	if( SDHI.SDSTS2.BIT.CBSY )  {				// Command Sequence End ?
		while( !SDHI.SDSTS2.BIT.SDCLKCREN )  ;		// Wait SD Bus non Busy
		SDHI.SDCLKCR.LONG = 0x0140;			// PCLKB/256
		while( SDHI.SDSTS2.BIT.CBSY )  ;		// Wait Command Sequence End
		while( !SDHI.SDSTS2.BIT.SDCLKCREN )  ;		// Wait SD Bus non Busy
		SDHI.SDCLKCR.LONG = 0x0340;			// PCLKB/256, Auto Clock
	}

	SDHI.SDARG = arg;					// Set Argument
	SDHI.SDCMD.LONG = cmd;					// Set Command
	ercd = tk_wai_flg( flgid, CMD_COMPLETE | CMD_TIMEOUT | SDHC_ERROR, TWF_ORW | TWF_BITCLR, &flgptn, WAIT_TIME );
	if( ercd < E_OK || ( flgptn & (SDHC_ERROR | CMD_TIMEOUT) ) )
		if( ercd == E_TMOUT || ( ercd == E_OK && flgptn & CMD_TIMEOUT ) )
			return E_TMOUT;
		else
			return E_IO;
	return E_OK;
}

LOCAL ER WaitForDMA(void)
{
ER ercd;
UINT flgptn;
	
	ercd = tk_wai_flg( flgid, TRANS_COMPLETE | SDHC_ERROR, TWF_ORW | TWF_BITCLR, &flgptn, WAIT_TIME );
	if( ercd < E_OK )
		return ercd;
	if( flgptn & SDHC_ERROR )  {
		SD_DMAC.DMCNT.BIT.DTE = 0;			// Transfer Stop
		IR( SDHI, SBFAI ) = 0;				// Clear Interrupt Flag
		return E_IO;
	}
	return E_OK;
}

EXPORT ER SDC_ReadBlock(void *buf, W start, SZ size)
{
ER ercd;

	if( ! SDCard.Exists || SDCard.CardType == UNKNOWN_CARD )// Check Card Type
		return E_NOMDA;
	if( buf == NULL || ((UW)buf & 0x3) )			// Check Parameter
		return E_PAR;
	if( SDCard.CardType == SD_CARD_STD )			// Standerd Card ?
		start *= BLOCK_SIZE;				// Convert to Byte Address
	SD_DMAC.DMSAR = (void*)&SDHI.SDBUFR;			// Set Source Address
	SD_DMAC.DMDAR = buf;					// Set Destination Address
	SD_DMAC.DMCRA = ( BLOCK_SIZE<<14 ) + ( BLOCK_SIZE>>2 );	// Set Transfer Count
	SD_DMAC.DMCRB = size;					// Set Block Count
	SD_DMAC.DMTMD.WORD = 0xA201;				// Set Transfer Mode
	SD_DMAC.DMAMD.WORD = 0x0080;				// Set Address Mode
	SD_DMAC.DMCNT.BIT.DTE = 1;				// Transfer Start
	if( size == 1 )						// Single Block Read
		ercd = SDC_SndCmd( READ_SINGLE_BLOCK, start );	// CMD17: READ_SINGLE_BLOCK
	else							// Multiple Block Read
		ercd = SDC_SndCmd( READ_MULT_BLOCK, start);	// CMD18: READ_MULT_BLOCK
	if( SDHI.SDRSP10 & OCR_ERRORBITS )
		ercd = E_IO;
	else
		ercd = WaitForDMA( );				// Wait DMA End
	return ercd;
}

EXPORT ER SDC_WriteBlock(void *buf, W start, SZ size)
{
ER ercd;

	if( ! SDCard.Exists || SDCard.CardType == UNKNOWN_CARD )// Check Card Type
		return E_NOMDA;
	else if( SDCard.Protect )				// Check Write Protect
		return E_RONLY;
	if( buf == NULL || ((UW)buf & 0x3) )			// Check Parameter
		return E_PAR;
	if( SDCard.CardType == SD_CARD_STD )			// Standerd Card ?
		start *= BLOCK_SIZE;				// Convert to Byte Address
	SD_DMAC.DMSAR = buf;					// Set Source Address
	SD_DMAC.DMDAR = (void *)&SDHI.SDBUFR;			// Set Destination Address
	SD_DMAC.DMCRA = ( BLOCK_SIZE<<14 ) + ( BLOCK_SIZE>>2 );	// Set Transfer Count
	SD_DMAC.DMCRB = size;					// Set Block Count
	SD_DMAC.DMTMD.WORD = 0xA201;				// Set Transfer Mode
	SD_DMAC.DMAMD.WORD = 0x8000;				// Set Address Mode
	SD_DMAC.DMCNT.BIT.DTE = 1;				// Transfer Start
	if( size == 1 )						// Single Block Write
		ercd = SDC_SndCmd( WRITE_SINGLE_BLOCK, start );	// CMD24: WRITE_SINGLE_BLOCK
	else
		ercd = SDC_SndCmd( WRITE_MULT_BLOCK, start );	// CMD25: WRITE_MULT_BLOCK
	if( ercd >= E_OK && !(SDHI.SDRSP10 & OCR_ERRORBITS) )  {
		ercd = WaitForDMA( );				// Wait For DMA End
		if( ercd == E_OK )
			do  {					// CMD13: SD_CMD_SEND_STATUS
				ercd = SDC_SndCmd( SEND_STATUS, SDCard.RCA );
				if( ercd == E_TMOUT )		// Card Status Error Check
					continue;
				else if( ercd != E_OK || SDHI.SDRSP10 & OCR_ERRORBITS )
					return E_IO;
			} while( 0x000000900 != (SDHI.SDRSP10 & 0x00001F00) )  ;
	}
	return ercd;
}

EXPORT ER SDC_CardInsert(void)
{
INT i;
ER ercd;
UINT res;

	while( !SDHI.SDSTS2.BIT.SDCLKCREN )  ;			// Wait SD Bus non Busy
	SDHI.SDCLKCR.LONG = 0x0140;				// PCLKB(60MHz)/256Å‡234.375KHz
//	Must be 74Cycle Provide					// 74Cycle/234.375KHz=315.7333ns
	for( i=0 ; i<9472  ; i++ )  ;				// 315.7333ns/120MHz*4cyc=9472
	while( !SDHI.SDSTS2.BIT.SDCLKCREN )  ;			// Wait SD Bus non Busy
	SDHI.SDCLKCR.LONG = 0x0340;				// PCLKB/256, Auto Clock

	ercd = SDC_SndCmd( GO_IDLE_STATE, 0 );			// CMD0: GO_IDLE_STATE
	if( ercd < E_OK )
		return ercd;
	ercd = SDC_SndCmd( SEND_IF_COND, 0x1AA );		// CMD8: SEND_IF_COND
	if( ercd < E_OK )					// Interface Error(Ver1.*) ?
		return ercd;
	SDCard.CardType = SD_CARD_STD;				// SD Card is 2.0 Standerd
	for( i=0 ; i<MAX_TRIAL ; i++ )  {
		ercd = SDC_SndCmd( APP_CMD, 0 );		// CMD55: APP_CMD
		if( ercd < E_OK || SDHI.SDRSP10 & OCR_ERRORBITS )
			return E_IO;
		ercd = SDC_SndCmd( SD_APP_OP_COND, 0x40FF8000 );// ACMD41: SD_APP_OP_COND
		if( ercd < E_OK )
			return E_IO;
		res = SDHI.SDRSP10;
		if( res & 0x80000000 )  {
			if( res & 0x40000000 )			// High Capacity ?
				SDCard.CardType = SD_CARD_HIGH;
			break;
		}
		tk_dly_tsk( 10 );
	}
	if( i == MAX_TRIAL )					// Time Out ?
		return E_TMOUT;
	SDCard.Exists = 1;					// SD Card Insert
	return E_OK;
}

EXPORT ER SDC_InitCard(void)
{
ER ercd;
UINT r1, r2, devsize, dsize_mult;
	
	ercd = SDC_SndCmd( ALL_SEND_CID, 0 );			// CMD2: ALL_SEND_CID
	if( ercd < E_OK )
		goto ERROR;
	ercd = SDC_SndCmd( SET_REL_ADDR, 0 );			// CMD3: SET_REL_ADDR
	if( ercd < E_OK )
		goto ERROR;
	if( SDHI.SDRSP10 & (R6_GENERAL_UNKNOWN | R6_ILLEGAL_CMD | R6_COM_CRC_FAILED) )
		goto ERROR;
	SDCard.RCA = SDHI.SDRSP10 & 0xFFFF0000;

	ercd = SDC_SndCmd( SEND_CSD, SDCard.RCA );		// CMD9: SEND_CSD
	if( ercd < E_OK )
		return E_IO;
	r1 = SDHI.SDRSP32;
	r2 = SDHI.SDRSP54;
	if( !(SDHI.SDRSP76 & 0x00C00000) )  {			// CSD Version 1.0
		devsize = ((r2 & 0x00000003) << 10) | (r1 >> 22);
		dsize_mult = (r1 & 0x00000380) >> 7;
		SDCard.BlockCount = (devsize + 1) * (1 << (dsize_mult + 2)) * (1 << (((r2 & 0x00000F00) >> 8) - 9));
	}
	else  {							// CSD Version 2.0
		devsize = ((r1 >> 8) & 0x003FFFFF);
		SDCard.BlockCount = (devsize + 1) << 10;
	}
	SDCard.BlockSize = BLOCK_SIZE;
	tm_printf("BlockCount = %d\n", SDCard.BlockCount);
	
	ercd = SDC_SndCmd( SEL_DESEL_CARD, SDCard.RCA );	// CMD7: SEL_DESEL_CARD
	if( ercd < E_OK || SDHI.SDRSP10 & OCR_ERRORBITS )	// Error Check
		goto ERROR;
	ercd = SDC_SndCmd( SET_BLOCKLEN, SDCard.BlockSize );	// CMD16: SET_BLOCKLEN
	if( ercd < E_OK || SDHI.SDRSP10 & OCR_ERRORBITS )	// Error Check
		goto ERROR;
	ercd = SDC_SndCmd( APP_CMD, SDCard.RCA );		// CMD55: APP_CMD
	if( ercd < E_OK || SDHI.SDRSP10 & OCR_ERRORBITS )	// Error Check
		goto ERROR;
	ercd = SDC_SndCmd( SET_BUS_WIDTH, 2 );			// ACMD6: SET_BUS_WIDTH
	if( ercd < E_OK || SDHI.SDRSP10 & OCR_ERRORBITS )	// Error Check
		goto ERROR;
	SDCard.Protect = SDHI.SDSTS1.BIT.SDWPMON;		// Set Write Protect
	return E_OK;
ERROR:
	SDCard.CardType = UNKNOWN_CARD;				// Set SD Card Type
	return E_IO;
}

EXPORT void SDC_CardReject(void)
{
	SDHI.SDRST.BIT.SDRST = 0;				// SDHI Reset
	tk_dly_tsk( 1 );					// Wait 1ms
	SDHI.SDRST.BIT.SDRST = 1;				// SDHI Reset Release
	SDCard.Exists = 0;					// SD Card Reject
}

EXPORT void SD_Detect_hdr(void)
{
UINT flgptn;
	flgptn = SDHI.SDSTS1.LONG << 7 & ( CARD_REJECT | CARD_INSERT );
	SDHI.SDSTS1.LONG &= 0xFFFFFFE7;				// Clear Interrupt Flag
	tk_set_flg( flgid, flgptn );				// Set EventFlag
}

EXPORT void SD_Int_hdr(void)
{
UINT flgptn;	
	flgptn = SDHI.SDSTS1.LONG << 7 & ( CMD_COMPLETE | TRANS_COMPLETE );
	SDHI.SDSTS1.LONG &= 0xFFFFFFFA;				// Clear Interrupt Flag
	flgptn += SDHI.SDSTS2.LONG & ( CMD_TIMEOUT | SDHC_ERROR );
	SDHI.SDSTS2.LONG &= 0xFFFFFF80;				// Clear Interrupt Flag
	tk_set_flg( flgid, flgptn );				// Set EventFlag
}

EXPORT void GroupBL1Handler(UINT dintno)
{
	if( IS( SDHI, CDETI ) )					// Occur SDHI CDETI Interrupt ?
		SD_Detect_hdr( );				// Call  SDHI CDETI Interrupt
	if( IS( SDHI, CACI ) )					// Occur SDHI CACI Interrupt ?
		SD_Int_hdr( );					// Call  SDHI CACI Interrupt
}

EXPORT ER SDC_Init(ID objid, T_DINT *p_dint)
{
	tk_dis_dsp( );						// Dispatch Disable
	if( ! MSTP( SDHI ) )  {					// SDHI is Already Enable ?
		tk_ena_dsp( );					// Dispatch Enable
		return E_OK;					// SDHI is Already Enable
	}
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( SDHI ) = 0;					// Enable SDHI
	MSTP( DMAC ) = 0;					// Enable DMAC
/* Disable SDRAM for AP-RX65N-0A */
	SYSTEM.SYSCR0.WORD = 0x5A01;				// Disable External Bus
/* Disable SDRAM for AP-RX65N-0A */
	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
	SDHI.SDRST.LONG = 0x0006;				// Reset SDHI

	MPC.PWPR.BIT.B0WI = 0;					// PFSWE Write Enable.
	MPC.PWPR.BIT.PFSWE = 1;					// PmnPFS Write Enable.
	MPC.PD5PFS.BYTE = 0x1A;					// Set SDHI_CLK Pin
	MPC.PD4PFS.BYTE = 0x1A;					// Set SDHI_CMD Pin
	MPC.PE6PFS.BYTE = 0x1A;					// Set SDHI_CD Pin
	MPC.PD6PFS.BYTE = 0x1A;					// Set SDHI_D0 Pin
	MPC.PD7PFS.BYTE = 0x1A;					// Set SDHI_D1 Pin
	MPC.PD2PFS.BYTE = 0x1A;					// Set SDHI_D2 Pin
	MPC.PD3PFS.BYTE = 0x1A;					// Set SDHI_D3 Pin
	MPC.PWPR.BYTE = 0x80;					// Write Disable

	PORTD.PMR.BYTE |= 0xFC;					// Enable Peripheral Pin
	PORTE.PMR.BIT.B6 = 1;
	tk_ena_dsp( );						// Dispatch Enable
	SDHI.SDRST.LONG = 0x0007;				// Release Reset
	tk_dly_tsk( 300 );					// Wait PCLKB*2^24(About 300ms)
	SDHI.SDDMAEN.LONG = 0x00001012;				// Enable DMA Transmission
	SDHI.SDIMSK2.LONG |= 0x00000300;			// Enable BREM Interrupt
	(&ICU.DMRSR0)[SD_CFG_DMA_CHANNEL*4] = VECT( SDHI, SBFAI );// Set Interrupt Factor
	DTCE( SDHI, SBFAI ) = 1;				// Set DTC Enable Bit
	IEN( SDHI, SBFAI ) = 1;					// Set Interrupt Enable Bit
	DMAC.DMAST.BIT.DMST = 1;				// DMA Master Enable
#ifdef __BIG
	SDHI.SDSWAP.LONG = 0x000000C0;				// Set Byte Swap Mode
#endif

	p_dint->intatr = TA_HLNG;				// Set Handler Attribute
#ifdef CLANGSPEC
	p_dint->inthdr = GroupBL1Handler;			// Set Handler Address
#else
	p_dint->inthdr = (FP)GroupBL1Handler;			// Set Handler Address
#endif
	tk_def_int( VECT( ICU, GROUPBL1 ), p_dint );		// Define Interrupt Handler

	EnableInt( VECT(ICU, GROUPBL1), SD_CFG_INT_PRIORITY );	// Enable BL1 Group Interrupt
	SDHI.SDIMSK1.LONG &= 0xFFFFFFFA;			// Enable RSPEND,ACEND Interrupt
	SDHI.SDIMSK2.LONG &= 0xFFFFFF80;			// Enable Error Interrupt
	EN( SDHI, CACI ) = 1;					// Enable SDHI CACI Group Interrupt
	SDHI.SDIMSK1.LONG &= 0xFFFFFFE7;			// Enable SDCDRM,SDCDIN Interrupt
	EN( SDHI, CDETI ) = 1;					// Enable SDHI CDETI Group Interrupt
	
	flgid = objid;						// Set Interface EventFlag ID
	return E_OK;
}

EXPORT ID SDC_GetTaskPri(void)
{
	return SD_CFG_TASK_PRIORITY;				// Return Task Priority
}