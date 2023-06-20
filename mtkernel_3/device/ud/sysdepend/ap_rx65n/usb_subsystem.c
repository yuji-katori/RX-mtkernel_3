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
#include "r_dmaca_rx_if.h"
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

EXPORT void R_DMACA_Init(void)
{
}

EXPORT dmaca_return_t R_DMACA_Open(uint8_t ch)
{
	tk_dis_dsp( );						// Dispatch Disable
	SYSTEM.PRCR.WORD = 0xA502;				// Protect Disable
	MSTP( DMAC ) = 0;					// Enable DMAC
	SYSTEM.SYSCR0.WORD = 0x5A01;				// Disable External Bus
	tk_ena_dsp( );						// Dispatch Enable
	DMAC.DMAST.BYTE = 0x01;					// Enable DMAC Activation
	return DMACA_SUCCESS;
}

EXPORT dmaca_return_t R_DMACA_Close(uint8_t ch)
{
	(&DMAC0.DMCNT.BYTE)[ch<<2] = 0x00;			// Disable DMA Transfers
	(&ICU.DMRSR0)[ch<<2] = 0x00;				// Clear DMRSRn
	return DMACA_SUCCESS;
}

dmaca_return_t R_DMACA_Int_Callback(uint8_t ch, void *callback)
{
	return DMACA_SUCCESS;
}

dmaca_return_t R_DMACA_Control(uint8_t ch, dmaca_command_t cmd, dmaca_stat_t *stat)
{
uint8_t  int_sts, req_sts;
volatile struct st_dmac1 __evenaccess *DMACA = (volatile struct st_dmac1 __evenaccess *)0x82000;
	DMACA += ch << 1;					// Adjust DMAC Channel Address
	switch( cmd )  {
	case DMACA_CMD_ENABLE:					// Enable DMA Transfer
		DMACA->DMCNT.BYTE = 0x01;			// Enable Transfer
		break;
	case DMACA_CMD_STATUS_GET:
		int_sts = DMACA->DMSTS.BYTE;			// Get DMAC status register
		req_sts = DMACA->DMREQ.BYTE;			// Get DMAC software Start register
		stat->act_stat  = ( int_sts & 0x80 ) != 0;	// Set Suspend Status
		stat->dtif_stat = ( int_sts & 0x10 ) != 0;	// Set End Interrupt status
		stat->esif_stat = ( int_sts & 0x01 ) != 0;	// Set Escape End Interrupt Status
		stat->soft_req_stat = ( req_sts & 0x01 ) != 0;	// Set Software Request Status
		stat->transfer_count = DMACA->DMCRB;		// Set Transfer Count
		break;
	case DMACA_CMD_DTIF_STATUS_CLR:
		DMACA->DMSTS.BIT.DTIF = 0;			// Clear Transfer Interrupt Flag
		break;
        }
	return DMACA_SUCCESS;
}

dmaca_return_t R_DMACA_Create(uint8_t ch, dmaca_transfer_data_cfg_t *cfg)
{
volatile struct st_dmac1 __evenaccess *DMACA = (volatile struct st_dmac1 __evenaccess *)0x82000;
	DMACA += ch << 1;					// Adjust DMAC Channel Address
	DMACA->DMREQ.BYTE = 0x00;				// Clear DMREQ
	DMACA->DMCNT.BYTE = 0x00;				// Disable DMA Transfers
	(&ICU.DMRSR0)[ch<<2] = cfg->act_source;			// Set DMRSR
    	DMACA->DMAMD.WORD = cfg->src_addr_mode | cfg->src_addr_repeat_area | cfg->des_addr_mode | cfg->des_addr_repeat_area;
								// Set DMAMD
	DMACA->DMTMD.WORD = cfg->transfer_mode | cfg->repeat_block_side | cfg->data_size | cfg->request_source;
								// Set DMTMD
	DMACA->DMSAR = cfg->p_src_addr;				// Set DMSAR
	DMACA->DMDAR = cfg->p_des_addr;				// Set DMSAR
	DMACA->DMCRA = cfg->block_size << 16 | cfg->block_size;	// Set DMCRA
	DMACA->DMCRB = cfg->transfer_count;			// Set DMCRB
	if( DMACA_CH0 == ch )
		DMAC0.DMOFR = cfg->offset_value;		// Set Offset Value
	DMACA->DMCSL.BYTE = cfg->interrupt_sel; 		// Set DMCSL
	DMACA->DMINT.BYTE = cfg->dtie_request | cfg->esie_request | cfg->rptie_request | cfg->sarie_request | cfg->darie_request;
	return DMACA_SUCCESS;					// Set DMINT
}

dmaca_return_t R_DMACA_Int_Enable(uint8_t ch, uint8_t  pri)
{
uint8_t vecnum;

	if( ch < 4 )  {
		vecnum = VECT( DMAC, DMAC0I ) + ch;	// DMAC0 ` DMAC3 Vector Number
		ICU.IPR[vecnum].BYTE = pri;		// Set Interrpt Priority Level
	}
//	else  {
//		vecnum = VECT( DMAC, DMAC74I );		// DMAC4 ` DMAC7 Vector Number
//		if( IPR( DMAC, DMAC74I ) < pri )	// Interrupt Level Check
//			IPR( DMAC, DMAC74I ) = pri;	// Set Interrupt Priority Level
//	}
	ICU.IER[vecnum>>3].BYTE |= 1<<(vecnum&7);	// Enable Interrupt
	return DMACA_SUCCESS;
}

dmaca_return_t R_DMACA_Int_Disable(uint8_t ch)
{
uint8_t vecnum;

	if( ch < 4 )
		vecnum = IPR( DMAC, DMAC0I ) + ch;	// DMAC0 ` DMAC3 Vector Number
//	else
//		vecnum = IPR( DMAC, DMAC74I );		// DMAC4 ` DMAC7 Vector Number
	ICU.IPR[vecnum].BYTE = 0;			// Clear Interrpt Priority Level
	ICU.IER[vecnum>>3].BYTE &= ~(1<<(vecnum&7));	// Disable Interrupt
	return DMACA_SUCCESS;
}