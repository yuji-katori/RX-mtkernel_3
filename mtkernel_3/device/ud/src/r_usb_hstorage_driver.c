/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hstorage_driver.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_hmsc.h"

#define	RETRY_COUNT	(10)

       uint16_t g_usb_hmsc_strg_process;

static uint8_t 	  usb_hmsc_read_partition_retry_count;
static uint8_t    usb_hmsc_drive_search_seq;
static uint16_t   usb_hmsc_root_devaddr;
static usb_cb_t   usb_shmsc_command_result;

static void       usb_hmsc_strg_drive_search_act(usb_utr_t *msg);
static void       usb_hmsc_strg_specified_path(usb_utr_t *msg);
static void       usb_hmsc_strg_check_result(usb_utr_t *msg, uint16_t data1, uint16_t data2);

/******************************************************************************
 Function Name   : usb_hmsc_strg_drive_search_act
 Description     : Storage drive search.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_strg_drive_search_act(usb_utr_t *msg)
{
uint16_t devadr;
	switch( usb_hmsc_drive_search_seq )  {
	case USB_SEQ_0:		// Get MAX_LUN
		USB_PRINTF0("\n*** Drive search !\n");
		g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_DRIVE_SEARCH;
		devadr = msg->keyword;
		if( USB_ERROR == usb_hmsc_ref_drvno( devadr ) )  {
			usb_hmsc_drive_search_seq = USB_SEQ_0;
			g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_DRIVE_SEARCH_END;
			usb_hmsc_strg_specified_path( msg );
			USB_NoSupportEvent( );			// Set USB Event(No Support)
		}
		else
			if( USB_QOVR == usb_hmsc_get_max_unit( devadr, g_usb_buf, usb_hmsc_strg_check_result ) )
				usb_hmsc_message_retry( USB_HSTRG_MBX, msg );
			else
				usb_hmsc_drive_search_seq++;
		break;
	case USB_SEQ_1:		// Check result
		if( USB_CTRL_END != msg->result )  {
			USB_PRINTF0("### GetMaxLUN error\n");
		}
		USB_PRINTF1(" Unit number is %d\n", g_usb_buf[0]);
		USB_PRINTF0("\nPlease wait device ready\n");
		usb_cpu_delay_xms( 300 );
		usb_hmsc_inquiry( );				// Inquiry
		usb_hmsc_drive_search_seq++;
		break;
	case USB_SEQ_2:		// Check result
		if( msg->result != USB_HMSC_OK )  {
			USB_PRINTF0("### Inquiry error\n");
		}
		usb_hmsc_read_format_capacity( );		// Read Format Capacity
		usb_hmsc_drive_search_seq++;
		break;
	case USB_SEQ_3:		// Read Capacity
		usb_hmsc_read_capacity( );
		usb_hmsc_drive_search_seq++;
		break;
	case USB_SEQ_4:
		if( msg->result != USB_HMSC_OK )  {
			usb_hmsc_test_unit( );			// TestUnitReady
			usb_hmsc_drive_search_seq++;
		}
		else  {
			usb_hmsc_drive_search_seq = USB_SEQ_6;	// Pass TestUnitReady
			usb_hmsc_read_partition_retry_count = 0;
			usb_hmsc_strg_specified_path( msg );
		}
		break;
	case USB_SEQ_5:
		if( msg->result != USB_HMSC_OK )
			usb_hmsc_test_unit( );			// TestUnitReady (Retry)
		else  {
			usb_hmsc_read_capacity( );		// Read Capacity (Retry) */
			usb_hmsc_read_partition_retry_count = 0;
			usb_hmsc_drive_search_seq++;
		}
		break;
        case USB_SEQ_6:
		usb_hmsc_read_partition_retry_count++;				// Update Retry count
		if( RETRY_COUNT == usb_hmsc_read_partition_retry_count )  {	// Chech Max Retry count
			usb_hmsc_drive_search_seq = USB_SEQ_0;			// Retry end
			g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_DRIVE_SEARCH_END;
			USB_NoSupportEvent( );					// Set USB Event(No Support)
		}
		usb_hmsc_drive_search_seq = USB_SEQ_0;
		g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_DRIVE_SEARCH_END;
		usb_hmsc_strg_specified_path( msg );
		break;
        default:
		usb_hmsc_drive_search_seq = USB_SEQ_0;
		g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_DRIVE_SEARCH_END;
		usb_hmsc_strg_specified_path( msg );
		USB_NoSupportEvent( );				// Set USB Event(No Support)
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_specified_path
 Description     : Next Process Selector.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_strg_specified_path(usb_utr_t *msg)
{
usb_utr_t *blk;
	blk = USB_GET_BLK( );				// Get Message Block
	blk->msginfo = g_usb_hmsc_strg_process;
	blk->keyword = msg->keyword;
	blk->result  = msg->result;
	USB_SND_MSG( USB_HSTRG_MBX, blk );		// Send Message
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_check_result
 Description     : Storage class check result.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
                 : uint16_t   data1	: Not used
                 : uint16_t   data2	: Not used
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_strg_check_result(usb_utr_t *msg, uint16_t data1, uint16_t data2)
{
	msg->result = msg->status;
	usb_hmsc_strg_specified_path( msg );
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_drive_task
 Description     : Storage drive task.
 Arguments       : usb_utr_t *msg	: Message Address
 Return value    : none
 ******************************************************************************/
void usb_hmsc_strg_drive_task(usb_utr_t *msg)
{
	switch( msg->msginfo )  {
	case USB_MSG_HMSC_STRG_DRIVE_SEARCH:
		usb_hmsc_strg_drive_search_act( msg );		// Start Drive search
		break;
	case USB_MSG_HMSC_STRG_DRIVE_SEARCH_END:
		g_usb_hmsc_strg_process = USB_NULL;
		if( USB_NULL != usb_shmsc_command_result )	// Check Callback function
			usb_shmsc_command_result( msg, usb_hmsc_root_devaddr, 0 );
		break;
	case USB_MSG_HMSC_STRG_RW_END:
		USB_StrgRWEndEvent( USB_HMSC_OK == msg->result ? USB_TRUE : USB_FALSE );
		break;						// Set USB Event(Strage R/W End)
	}
	USB_REL_BLK( msg );					// Releasse Message Block
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_drive_search
 Description     : Searches drive SndMsg.
 Arguments       : uint16_t devadr	: Device Address
                 : usb_cb_t complete	: Callback function
 Return value    : none
 ******************************************************************************/
void usb_hmsc_strg_drive_search(uint16_t devadr, usb_cb_t complete)
{
usb_utr_t *msg;
	usb_hmsc_root_devaddr = devadr;
	usb_shmsc_command_result = complete;
	usb_hmsc_drive_search_seq = USB_SEQ_0;
	msg = USB_GET_BLK( );				// Get Message Block
	msg->msginfo = USB_MSG_HMSC_STRG_DRIVE_SEARCH;
	msg->keyword = devadr;
	msg->complete = complete;
	USB_SND_MSG( USB_HSTRG_MBX, msg );		// Send Message
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_drive_open
 Description     : Releases drive.
 Arguments       : uint16_t devadr	: Device Address
 Return value    : USB_OK/USB_ERROR
 ******************************************************************************/
uint8_t usb_hmsc_strg_drive_open(uint16_t devadr)
{
	if( USB_ERROR == usb_hmsc_pipe_info( g_usb_hmsc_interface_table, g_usb_hmsc_speed, g_usb_hmsc_config_table[2] ) )  {
		USB_PRINTF0("### Device information error !\n");	// Pipe Information table set
		return USB_ERROR;
	}
	g_drive_search_que = devadr;
	g_drive_search_que_cnt++;
	if( USB_FALSE == g_drive_search_lock )  {
		g_drive_search_lock = g_drive_search_que;
		devadr = g_drive_search_lock & USB_ADDRESS_MASK;
		usb_hmsc_strg_drive_search( devadr, usb_hmsc_drive_complete );
	}
	return USB_OK;
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_drive_close
 Description     : Releases drive.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hmsc_strg_drive_close(void)
{
	g_usb_hmsc_out_pipectr = 0;			// Toggle clear
	g_usb_hmsc_in_pipectr = 0;			// Toggle clear
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_read_sector
 Description     : Releases drive.
 Arguments       : uint8_t  *buf	: Buffer address
                 : uint32_t  secno	: Sector number
                 : uint16_t  seccnt	: Sector count
                 : uint32_t  trans_byte	: Transfer size
 Return value    : uint16_t		: [DONE/ERROR]
 ******************************************************************************/
uint16_t usb_hmsc_strg_read_sector(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte)
{
	g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_RW_END;
	usb_hmsc_read10( buf, secno, seccnt, trans_byte );
	return USB_OK;
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_write_sector
 Description     : Writes sector information.
 Arguments       : uint8_t  *buf	: Buffer address
                 : uint32_t  secno	: Sector number
                 : uint16_t  seccnt	: Sector count
                 : uint32_t  trans_byte	: Transfer size
 Return value    : uint16_t		: [DONE/ERROR]
 ******************************************************************************/
uint16_t usb_hmsc_strg_write_sector(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte)
{
	g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_RW_END;
	usb_hmsc_write10( buf, secno, seccnt, trans_byte );
	return USB_OK;
}

/******************************************************************************
 Function Name   : usb_hmsc_strg_write_sector
 Description     : Writes sector information.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hmsc_strg_read_capacity(void)
{
	g_usb_hmsc_strg_process = USB_MSG_HMSC_STRG_RW_END;
	usb_hmsc_read_capacity( );
}

/******************************************************************************
 Function Name   : USB_GetCapacity
 Description     : Get USB blockcount and USB blocksize.
 Arguments       : unsigned int *BlockCount	: Pointer of Block Count
                 : unsigned int *BlockSize	: Pointer of Block Size
 Return value    : none
 ******************************************************************************/
void USB_GetCapacity(unsigned int *BlockCount, unsigned int *BlockSize)
{
	*BlockCount = (g_usb_buf[0]<<24) + (g_usb_buf[1]<<16) + (g_usb_buf[2]<<8) + g_usb_buf[3];
	*BlockSize  = (g_usb_buf[4]<<24) + (g_usb_buf[5]<<16) + (g_usb_buf[6]<<8) + g_usb_buf[7];
}
