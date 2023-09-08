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

#include "r_usb_typedef.h"

#define USB_IDMAX		(5)		// Maximum Task ID + 1
#define	USB_PRIMAX		(4)		// Maximum Priority number + 1
#define	USB_TABLEMAX		(USB_BLKMAX)	// Maximum priority table

// Fixed-sized memory blocks
usb_utr_t  g_usb_scheduler_block[USB_BLKMAX];
uint8_t    g_usb_buf[USB_BUFSIZE];

// Priority Table
usb_utr_t *usb_scheduler_table_dt[USB_PRIMAX][USB_TABLEMAX];
uint8_t    usb_scheduler_table_id[USB_PRIMAX][USB_TABLEMAX];
uint8_t    usb_scheduler_pri_r[USB_PRIMAX];
uint8_t    usb_scheduler_pri_w[USB_PRIMAX];
uint8_t    usb_scheduler_pri[USB_IDMAX];
// Schedule Set Flag
uint8_t    g_usb_scheduler_id_use;
usb_utr_t *g_usb_scheduler_dt_use;
// Fixed-sized memory pools
static usb_utr_t *usb_scheduler_pool;

/******************************************************************************
 Function Name   : usb_cstd_snd_msg
 Description     : Send a message to the specified id (mailbox) while executing 
                   an interrupt.
 Argument        : uint8_t    id	: ID number (mailbox)
                 : usb_utr_t *msg	: Message pointer
 Return          : none
 ******************************************************************************/
void usb_cstd_snd_msg(uint8_t id, usb_utr_t *msg)
{
uint8_t usb_pri;				// Task Priority
uint8_t usb_write;				// Priority Table Writing pointer
	usb_pri   = usb_scheduler_pri[id];	// Read priority and table pointer
	usb_write = usb_scheduler_pri_w[usb_pri];
	usb_write++;				// Renewal write pointer
	if( usb_write == USB_TABLEMAX )
		usb_write = USB_TABLECLR;
	if( id == USB_HCD_MBX - 1 )
		id = USB_HCD_MBX;				// Adjust Mailbox ID
	usb_scheduler_table_id[usb_pri][usb_write] = id;	// Set priority table
	usb_scheduler_table_dt[usb_pri][usb_write] = msg;
	usb_scheduler_pri_w[usb_pri] = usb_write;
}

/******************************************************************************
 Function Name   : usb_cstd_get_blk
 Description     : Get a memory block for the caller.
 Argument        : uint8_t    id	: ID number (mailbox).
 Return          : usb_utr_t *blk	: Memory block pointer.
 ******************************************************************************/
usb_utr_t *usb_cstd_get_blk(void)
{
usb_utr_t *blk;
	blk = usb_scheduler_pool;		// Set Block Address
	usb_scheduler_pool = blk->msghead;	// Set Next Block Address
	return blk;
}

/******************************************************************************
 Function Name   : usb_cstd_rel_blk
 Description     : Release a memory block.
 Argument        : usb_utr_t *blk	: Memory block pointer.
 Return          : none
 ******************************************************************************/
void usb_cstd_rel_blk(usb_utr_t *blk)
{
	blk->msghead = usb_scheduler_pool;	// Set Next Block Address
	usb_scheduler_pool = blk;		// Set Block Address
}

/******************************************************************************
 Function Name   : usb_cstd_sche_init
 Description     : Scheduler initialization.
 Argument        : none
 Return          : none
 ******************************************************************************/
void usb_cstd_sche_init(void)
{
uint8_t i;
	for( i=0 ; i<USB_BLKMAX ; i++ )  {
		g_usb_scheduler_block[i].msghead = usb_scheduler_pool;
		usb_scheduler_pool = &g_usb_scheduler_block[i];
	}
}

/******************************************************************************
 Function Name   : usb_cstd_scheduler
 Description     : The scheduler.
 Argument        : none
 Return          : USB_OK(Exist Message)/USB_ERROR(Not Exist Message)
 ******************************************************************************/
uint8_t usb_cstd_scheduler(void)
{
uint8_t usb_pri;				// Priority Counter
uint8_t usb_read;				// Priority Table read pointer
	for( usb_pri=0 ; usb_pri<USB_PRIMAX ; usb_pri++ )  {
		usb_read = usb_scheduler_pri_r[usb_pri];
		if( usb_read != usb_scheduler_pri_w[usb_pri] )  {
			usb_read++;		// Priority Table read pointer increment
			if( usb_read == USB_TABLEMAX )
				usb_read = USB_TABLECLR;
			g_usb_scheduler_id_use = usb_scheduler_table_id[usb_pri][usb_read];
			g_usb_scheduler_dt_use = usb_scheduler_table_dt[usb_pri][usb_read];
			usb_scheduler_pri_r[usb_pri] = usb_read;
			return USB_OK;
		}
	}
	return USB_ERROR;
}

/******************************************************************************
 Function Name   : usb_cstd_set_task_pri
 Description     : Set a task's priority.
 Argument        : uint8_t taskid	: Task id.
                 : uint8_t pri		: The task priority to be set.
 Return          : none
 ******************************************************************************/
void usb_cstd_set_task_pri(uint8_t taskid, uint8_t pri)
{
	usb_scheduler_pri[taskid] = pri;
}
