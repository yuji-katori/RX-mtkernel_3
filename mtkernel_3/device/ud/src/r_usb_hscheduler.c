/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hscheduler.c
 */

#include "r_usb_basic_if.h"
#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_hmsc.h"
#include "dev_ud.h"

#if ((USB_CFG_DTC == USB_CFG_ENABLE) || (USB_CFG_DMA == USB_CFG_ENABLE))
#include "r_usb_dmac.h"
#endif  /* ((USB_CFG_DTC == USB_CFG_ENABLE) || (USB_CFG_DMA == USB_CFG_ENABLE)) */

#if (BSP_CFG_RTOS_USED == 0)    /* Non-OS */
#if ((USB_CFG_MODE & USB_CFG_HOST) == USB_CFG_HOST)
/******************************************************************************
 Macro definitions
 ******************************************************************************/
#define USB_IDMAX           (4u)                  /* Maximum Task ID +1 */
#define USB_PRIMAX          (3u)                  /* Maximum Priority number +1 */
#define USB_BLKMAX          (4u)                  /* Maximum block */
#define USB_TABLEMAX        (USB_BLKMAX)          /* Maximum priority table */
#define USB_WAIT_EVENT_MAX  (2u)

/******************************************************************************
 Private global variables and functions
 ******************************************************************************/
/* Priority Table */
static usb_msg_t* p_usb_scheduler_table_add[USB_PRIMAX][USB_TABLEMAX];
static uint8_t usb_scheduler_table_id[USB_PRIMAX][USB_TABLEMAX];
static uint8_t usb_scheduler_pri_r[USB_PRIMAX];
static uint8_t usb_scheduler_pri_w[USB_PRIMAX];
static uint8_t usb_scheduler_pri[USB_IDMAX];

/* Schedule Set Flag  */
static uint8_t usb_scheduler_schedule_flag;

/* Fixed-sized memory pools */
static usb_utr_t usb_scheduler_block[USB_BLKMAX];
static uint8_t usb_scheduler_blk_flg[USB_BLKMAX];

static usb_msg_t* p_usb_scheduler_add_use;
static uint8_t usb_scheduler_id_use;
/******************************************************************************
 Exported global variables
******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_rec_msg
 Description     : Receive a message to the specified id (mailbox).
 Argument        : uint8_t      id      : ID number (mailbox).
                 : usb_msg_t**  mess    : Message pointer
                 : usb_tm_t     tm      : Timeout Value
 Return          : uint16_t             : USB_OK / USB_ERROR
 ******************************************************************************/
usb_er_t usb_cstd_rec_msg(uint8_t id, usb_msg_t** mess, usb_tm_t tm)
{
	if ( id == usb_scheduler_id_use )  {
		*mess = p_usb_scheduler_add_use;
		return USB_OK;
	}
	return USB_ERROR;
}
/******************************************************************************
 End of function usb_cstd_rec_msg
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_snd_msg
 Description     : Send a message to the specified id (mailbox).
 Argument        : uint8_t      id      : ID number (mailbox).
                 : usb_msg_t*   mess    : Message pointer
 Return          : usb_er_t             : USB_OK / USB_ERROR
 ******************************************************************************/
usb_er_t usb_cstd_snd_msg(uint8_t id, usb_msg_t* mess)
{
usb_er_t status;

	usb_cpu_int_disable( );			/* USB interrupt disable */
	status = usb_cstd_isnd_msg(id, mess);
	usb_cpu_int_enable();			/* USB interrupt enable */
	return status;
}
/******************************************************************************
 End of function usb_cstd_snd_msg
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_isnd_msg
 Description     : Send a message to the specified id (mailbox) while executing 
                 : an interrupt.
 Argument        : uint8_t      id      : ID number (mailbox).
                 : usb_msg_t*   mess    : Message pointer
 Return          : usb_er_t             : USB_OK / USB_ERROR
 ******************************************************************************/
usb_er_t usb_cstd_isnd_msg(uint8_t id, usb_msg_t* mess)
{
uint8_t usb_pri;			/* Task Priority */
uint8_t usb_write;			/* Priority Table Writing pointer */

	/* Read priority and table pointer */
	usb_pri = usb_scheduler_pri[id];
	usb_write = usb_scheduler_pri_w[usb_pri];
	if( usb_pri < USB_PRIMAX )  {
		/* Renewal write pointer */
		usb_write++;
		if( usb_write >= USB_TABLEMAX )
			usb_write = USB_TBLCLR;
		/* Check pointer */
		if( usb_write == usb_scheduler_pri_r[usb_pri] )
			goto Error;
		/* Save message */
		/* Set priority table */
		usb_scheduler_table_id[usb_pri][usb_write] = id;
		p_usb_scheduler_table_add[usb_pri][usb_write] = mess;
		usb_scheduler_pri_w[usb_pri] = usb_write;
		USB_SetEvent( );
		return USB_OK;
	}
Error:
	USB_PRINTF0("SND_MSG ERROR !!\n");
	while( 1 )  ;
	return USB_ERROR;
}
/******************************************************************************
 End of function usb_cstd_isnd_msg
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_pget_blk
 Description     : Get a memory block for the caller.
 Argument        : uint8_t      id      : ID number (mailbox).
                 : usb_utr_t**  blk     : Memory block pointer.
 Return          : usb_er_t             : USB_OK / USB_ERROR
 ******************************************************************************/
usb_er_t usb_cstd_pget_blk(uint8_t id, usb_utr_t** blk)
{
uint8_t usb_s_pblk_c;

	for( usb_s_pblk_c = USB_CNTCLR ; USB_BLKMAX != usb_s_pblk_c ; usb_s_pblk_c++ )
		if( USB_FLGCLR == usb_scheduler_blk_flg[usb_s_pblk_c] )  {
			/* Acquire fixed-size memory block */
			*blk = &usb_scheduler_block[usb_s_pblk_c];
			usb_scheduler_blk_flg[usb_s_pblk_c] = USB_FLGSET;
			return USB_SUCCESS;
		}
	/* Error of BLK Table Full !!  */
	USB_PRINTF1("usb_scBlkFlg[%d][] Full !!\n",id);
	while( 1 )  ;
	return USB_ERR_NG;
}
/******************************************************************************
 End of function usb_cstd_pget_blk
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_rel_blk
 Description     : Release a memory block.
 Argument        : uint8_t      id      : ID number (mailbox).
                 : usb_utr_t*   blk     : Memory block pointer.
 Return          : usb_er_t             : USB_OK / USB_ERROR
 ******************************************************************************/
usb_er_t usb_cstd_rel_blk(uint8_t id, usb_utr_t* blk)
{
uint16_t usb_s_rblk_c;

	for( usb_s_rblk_c = USB_CNTCLR ; USB_BLKMAX != usb_s_rblk_c ; usb_s_rblk_c++ )
		if( &usb_scheduler_block[usb_s_rblk_c] == blk )  {
			/* Release fixed-size memory block */
			usb_scheduler_blk_flg[usb_s_rblk_c] = USB_FLGCLR;
			return USB_SUCCESS;
		}
	/* Error of BLK Flag is not CLR !!  */
	USB_PRINTF0("TskBlk NO CLR !!\n");
	while( 1 )  ;
	return USB_ERR_NG;
}
/******************************************************************************
 End of function usb_cstd_rel_blk
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_wai_msg
 Description     : Runs USB_SND_MSG after running the scheduler the specified 
                 : number of times.
 Argument        : uint8_t      id      : ID number (mailbox).
                 : usb_msg_t    *mess   : Message pointer.
                 : uint16_t     times   : Timeout value.
 Return          : usb_er_t             : USB_OK / USB_ERROR.
 ******************************************************************************/
usb_err_t usb_cstd_wai_msg(uint8_t id, usb_msg_t* mess, usb_tm_t times)
{
	/* Error !!  */
	USB_PRINTF0("WAI_MSG ERROR !!\n");
	while( 1 )  ;
	return USB_ERR_NG;
}
/******************************************************************************
 End of function usb_cstd_wai_msg
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_sche_init
 Description     : Scheduler initialization.
 Argument        : none
 Return          : none
 ******************************************************************************/
void usb_cstd_sche_init(void)
{
uint8_t i, j;

	for( i=0 ; USB_PRIMAX != i ; i++ )
		for( j=0 ; USB_TABLEMAX != j ; j++ )
			usb_scheduler_table_id[i][j] = USB_IDMAX;
	for( i=0 ; USB_IDMAX != i; i++ )
		usb_scheduler_pri[i] = USB_IDCLR;
}
/******************************************************************************
 End of function usb_cstd_sche_init
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_scheduler
 Description     : The scheduler.
 Argument        : none
 Return          : none
 ******************************************************************************/
void usb_cstd_scheduler(void)
{
uint8_t usb_pri;			/* Priority Counter */
uint8_t usb_read;			/* Priority Table read pointer */

	for( usb_pri = USB_CNTCLR ; usb_pri < USB_PRIMAX ; usb_pri++ )  {
		usb_read = usb_scheduler_pri_r[usb_pri];
		if( usb_read != usb_scheduler_pri_w[usb_pri] )  {
			/* Priority Table read pointer increment */
			usb_read++;
			if( usb_read >= USB_TABLEMAX )
				usb_read = USB_TBLCLR;
			/* Set practice message */
			usb_scheduler_id_use = usb_scheduler_table_id[usb_pri][usb_read];
			p_usb_scheduler_add_use = p_usb_scheduler_table_add[usb_pri][usb_read];
			usb_scheduler_table_id[usb_pri][usb_read] = USB_IDMAX;
			usb_scheduler_pri_r[usb_pri] = usb_read;
			usb_scheduler_schedule_flag = USB_FLGSET;
			break;
		}
	}
#if ((USB_CFG_DTC == USB_CFG_ENABLE) || (USB_CFG_DMA == USB_CFG_ENABLE))
	usb_cstd_dma_driver( );			/* USB DMA driver */
#endif  /* ((USB_CFG_DTC == USB_CFG_ENABLE) || (USB_CFG_DMA == USB_CFG_ENABLE)) */
}
/******************************************************************************
 End of function usb_cstd_scheduler
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_set_task_pri
 Description     : Set a task's priority.
 Argument        : uint8_t tasknum   : Task id.
                 : uint8_t pri       : The task priority to be set.
 Return          : none
 ******************************************************************************/
void usb_cstd_set_task_pri(uint8_t tasknum, uint8_t pri)
{
	usb_scheduler_pri[tasknum] = pri;
}
/******************************************************************************
 End of function usb_cstd_set_task_pri
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_check_schedule
 Description     : Check schedule flag to see if caller's "time has come", then 
                 : clear it.
 Argument        : none
 Return          : flg   : usb_scheduler_schedule_flag
 ******************************************************************************/
uint8_t usb_cstd_check_schedule (void)
{
uint8_t flg;

	flg = usb_scheduler_schedule_flag;
	USB_ClearEvent( flg );
	usb_scheduler_schedule_flag = USB_FLGCLR;
	return flg;
}
/******************************************************************************
 End of function usb_cstd_check_schedule
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_cstd_check_schedule
 Description     : Check Maixbox
                 : clear it.
 Argument        : none
 Return          : none
 ******************************************************************************/
void usb_cstd_check_mailbox(void)
{
uint8_t i;
	for( i=0 ; i<USB_PRIMAX ; i++ )
		if( usb_scheduler_pri_r[i] != usb_scheduler_pri_w[i] )  {
			USB_SetEvent( );
			break;
		}
}
/******************************************************************************
 End of function usb_cstd_check_schedule
 ******************************************************************************/

/******************************************************************************
 Function Name   : usb_hmsc_strg_wait_cmdend
 Description     : wait fot hmsc strg commnad end
 Argument        : uint8_t pdrv      : Physical drive number.
 Return          : usb_er_t          : USB_OK / USB_ERROR.
******************************************************************************/
usb_er_t usb_hmsc_strg_wait_cmdend(uint8_t side)
{
uint16_t res, err;
usb_utr_t *mess;

	do {
		usb_cstd_check_mailbox( );		/* Check Mailbox */
		res = R_USB_HmscGetDevSts( side );	/* Check Detach */
		if( usb_cstd_check_schedule( ) == USB_FLGSET )  {
			usb_hstd_hcd_task( 0 );		/* HCD  Task */
			usb_hstd_mgr_task( 0 );		/* MGR  Task */
			usb_hmsc_task( );		/* HMSC Task */
		}
		usb_cstd_scheduler( );
		if( usb_scheduler_schedule_flag != USB_FLGSET )
			USB_WaiEvent( );		/* Wait USB Event */
		err = USB_TRCV_MSG( USB_HSTRG_MBX, (usb_msg_t **)&mess, 0 );	/* Receive write complete msg */
	}  while( err != USB_OK && res != USB_FALSE )  ;

	if( USB_OK == err )				/* Complete Hmsc Strg Command */
		USB_REL_BLK( USB_HSTRG_MPL, (usb_mh_t)mess );
	return err;
}
/******************************************************************************
 End of function usb_hmsc_strg_wait_cmdend
 ******************************************************************************/

#endif  /* (USB_CFG_MODE & USB_CFG_HOST) == USB_CFG_HOST */

#endif  /* BSP_CFG_RTOS_USED == 0 */

/******************************************************************************
 End  Of File
 ******************************************************************************/
