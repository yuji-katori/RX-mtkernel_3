/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hmsc_driver.c
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"
#include "r_usb_hmsc.h"
#include "r_usb_bitdefine.h"

static uint16_t usb_hmsc_data_act(usb_utr_t *msg);
static void     usb_hmsc_stall_err(usb_utr_t *msg);
static void     usb_hmsc_specified_path(uint16_t keyword, uint16_t result);
static void     usb_hmsc_check_result(usb_utr_t *msg, uint16_t data1, uint16_t data2);
static void     usb_hmsc_command_result(usb_utr_t *msg, uint16_t result);
static void     usb_hmsc_get_string_desc(uint16_t string);
static uint16_t usb_hmsc_send_cbw(usb_utr_t *msg);
static uint16_t usb_hmsc_send_cbw_check(uint16_t result);
static uint16_t usb_hmsc_send_data(uint8_t *buf, uint32_t size);
static uint16_t usb_hmsc_send_data_check(uint16_t result);
static uint16_t usb_hmsc_get_data(uint8_t *buf, uint32_t size);
static uint16_t usb_hmsc_get_data_check(uint16_t result);
static uint16_t usb_hmsc_get_csw(void);
static uint16_t usb_hmsc_get_csw_check(usb_utr_t *msg, uint16_t result);
static void     usb_hmsc_clear_stall(usb_cb_t complete);
static uint16_t usb_hmsc_std_req_check(uint16_t result);
static usb_er_t usb_hmsc_mass_storage_reset(uint16_t devadr, usb_cb_t complete);
static void     usb_hmsc_set_els_cbw(uint8_t *cbwcb, uint32_t trans_byte);
static void     usb_hmsc_set_rw_cbw(uint16_t command, uint32_t secno, uint16_t seccnt, uint32_t trans_byte);
static void     usb_hmsc_clr_data(uint8_t *buf);
static void     usb_hmsc_no_data(void);
static void     usb_hmsc_data_in(uint8_t *buf, uint32_t size);
static void     usb_hmsc_data_out(uint8_t *buf, uint32_t size);
static void     usb_hmsc_class_check_result(usb_utr_t *msg, uint16_t data1, uint16_t data2);
static void     usb_hmsc_enumeration(usb_utr_t *msg);

static uint8_t              usb_hmsc_data_seq;
static uint8_t              usb_hmsc_stall_err_seq;
static uint8_t              usb_hmsc_init_seq;
static uint8_t              usb_hmsc_csw_err_loop;
static uint16_t             usb_hmsc_process;
static uint32_t             usb_hmsc_trans_size;
static uint32_t             usb_hmsc_cmd_data_length;
static uint8_t		   *usb_hmsc_buf;
static uint8_t		   *usb_hmsc_device_table;
static usb_utr_t            usb_hmsc_trans_data;
static usb_utr_t            usb_hmsc_receive_data;
static usb_ctrl_trans_t     trans_table;
static uint8_t              usb_cbwcb[12];
static uint32_t             usb_hmsc_csw_tag_no;
static usb_pipe_table_reg_t usb_hmsc_pipe_table[USB_PIPE_DIR_MAX];

uint8_t        g_drive_search_que_cnt;
uint8_t        g_drive_search_que;
uint8_t        g_drive_search_lock;
uint8_t        g_usb_hmsc_sub_class;
uint16_t       g_usb_hmsc_speed;
uint16_t       g_usb_hmsc_out_pipectr;
uint16_t       g_usb_hmsc_in_pipectr;
uint8_t       *g_usb_hmsc_config_table;
uint8_t       *g_usb_hmsc_interface_table;
usb_msc_cbw_t  g_usb_hmsc_cbw;
usb_msc_csw_t  g_usb_hmsc_csw;

/******************************************************************************
 Function Name   : usb_hmsc_task
 Description     : USB HMSC Task.
 Arguments       : usb_utr_t *msg	: Message Address
 Return value    : none
 ******************************************************************************/
void usb_hmsc_task(usb_utr_t *msg)
{
	switch( msg->msginfo )  {
	case USB_MSG_CLS_INIT:
		usb_hmsc_enumeration( msg );
		break;
	case USB_MSG_HMSC_NO_DATA:
	case USB_MSG_HMSC_DATA_IN:
	case USB_MSG_HMSC_DATA_OUT:
		usb_hmsc_data_act( msg );
		break;
	case USB_MSG_HMSC_CBW_ERR:
	case USB_MSG_HMSC_CSW_PHASE_ERR:
		usb_hmsc_stall_err( msg );
		break;
	}
	USB_REL_BLK( msg );				// Release Message Block
}

/******************************************************************************
 Function Name   : usb_hmsc_enumeration
 Description     : check class.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_enumeration(usb_utr_t *msg)
{
uint16_t retval;
uint16_t iproduct;
uint16_t result = USB_OK;
	switch( usb_hmsc_init_seq )  {
	case USB_SEQ_0:			// Check Interface Descriptor (deviceclass)
		if( USB_IFCLS_MAS != g_usb_hmsc_interface_table[USB_IF_B_INTERFACECLASS] )  {
			USB_PRINTF1("### Interface deviceclass is %x , not support.\n",
                        g_usb_hmsc_interface_table[USB_IF_B_INTERFACECLASS]);
			result = USB_ERROR;
		}			// Check Interface Descriptor (subclass)
		g_usb_hmsc_sub_class = g_usb_hmsc_interface_table[USB_IF_B_INTERFACESUBCLASS];
		if( USB_ATAPI == g_usb_hmsc_sub_class )  {
			USB_PRINTF0(" Interface subclass  : SFF-8070i\n");
		}
		else if( USB_SCSI == g_usb_hmsc_sub_class )  {
			USB_PRINTF0(" Interface subclass  : SCSI transparent command set\n");
		}
		else if( USB_ATAPI_MMC5 == g_usb_hmsc_sub_class )  {
			USB_PRINTF0(" Interface subclass  : ATAPI command set\n");
		}
		else  {
			USB_PRINTF1("### Interface subclass is %x , not support.\n", g_usb_hmsc_sub_class);
			result = USB_ERROR;
		}			// Check Interface Descriptor (protocol)
		if( USB_BOTP == g_usb_hmsc_interface_table[USB_IF_B_INTERFACEPROTOCOL] )  {
			USB_PRINTF0(" Interface protocol  : BOT \n");
		}
		else  {
			USB_PRINTF1("### Interface protocol is %x , not support.\n",
                        g_usb_hmsc_interface_table[USB_IF_B_INTERFACEPROTOCOL]);
			result = USB_ERROR;
		}			// Check Endpoint number
		if( g_usb_hmsc_interface_table[USB_IF_B_NUMENDPOINTS] < 2 )  {
			USB_PRINTF1("### Endpoint number is %x , less than 2.\n",
                        g_usb_hmsc_interface_table[USB_IF_B_NUMENDPOINTS]);
			result = USB_ERROR;
		}			// Send GetDescriptor(Stirng)
		if( USB_ERROR != result )  {
			usb_hmsc_get_string_desc( 0 );
			usb_hmsc_init_seq++;
		}
		break;
	case USB_SEQ_1:
		retval = usb_hmsc_std_req_check( msg->result );
		if( USB_ERROR == retval )
			result = USB_ERROR;
		else  {			// Send GetDescriptor(Stirng)
			iproduct = usb_hmsc_device_table[USB_DEV_I_PRODUCT];
			usb_hmsc_get_string_desc( iproduct );
			usb_hmsc_init_seq++;
		}
		break;
	case USB_SEQ_2:
		retval = usb_hmsc_std_req_check( msg->result );
		if( USB_ERROR == retval )
			result = USB_ERROR;
		if( USB_ERROR != result )  {		// Return to MGR
			usb_hstd_return_enu_mgr( retval );
			usb_hmsc_init_seq = USB_SEQ_0;
		}
		break;
	default:
		result = USB_ERROR;
		break;
	}
	if( USB_ERROR == result )  {			// Return to MGR
		usb_hstd_return_enu_mgr( USB_ERROR );
		usb_hmsc_init_seq = USB_SEQ_0;
		USB_NoSupportEvent( );			// Set USB Event(No Support)
	}
}

/******************************************************************************
 Function Name   : usb_hmsc_set_rw_cbw
 Description     : CBW parameter initialization for the READ10/WRITE10 command.
 Arguments       : uint16_t command	: ATAPI command
                 : uint32_t secno	: Sector number
                 : uint16_t seccnt	: Sector count
                 : uint32_t trans_byte	: Transfer size
 Return value    : none
 ******************************************************************************/
void usb_hmsc_set_rw_cbw(uint16_t command, uint32_t secno, uint16_t seccnt, uint32_t trans_byte)
{
	// CBW parameter set
	g_usb_hmsc_cbw.dcbw_tag = usb_hmsc_csw_tag_no;
	g_usb_hmsc_cbw.dcbw_dtl_lo = trans_byte;
	g_usb_hmsc_cbw.dcbw_dtl_ml = trans_byte >> 8;
	g_usb_hmsc_cbw.dcbw_dtl_mh = trans_byte >> 16;
	g_usb_hmsc_cbw.dcbw_dtl_hi = trans_byte >> 24;
	g_usb_hmsc_cbw.bm_cbw_flags = 0;
	g_usb_hmsc_cbw.bcbw_lun = 0;
	g_usb_hmsc_cbw.bcbwcb_length = 0;
	g_usb_hmsc_cbw.cbwcb[0] = command;		// ATAPI_COMMAND
	g_usb_hmsc_cbw.cbwcb[1] = 0x00;		// LUN
	g_usb_hmsc_cbw.cbwcb[2] = secno >> 24;	// sector address
	g_usb_hmsc_cbw.cbwcb[3] = secno >> 16;
	g_usb_hmsc_cbw.cbwcb[4] = secno >> 8;
	g_usb_hmsc_cbw.cbwcb[5] = secno;
	g_usb_hmsc_cbw.cbwcb[6] = 0x00;		// Reserved
	g_usb_hmsc_cbw.cbwcb[7] = seccnt >> 8;	// Sector length
	g_usb_hmsc_cbw.cbwcb[8] = seccnt;		// Block address
	g_usb_hmsc_cbw.cbwcb[9] = 0x00;		// Control data
	// ATAPI command check
	switch( command )  {
	case USB_ATAPI_TEST_UNIT_READY:
	case USB_ATAPI_REQUEST_SENSE:
	case USB_ATAPI_INQUIRY:
	case USB_ATAPI_MODE_SELECT6:
	case USB_ATAPI_MODE_SENSE6:
	case USB_ATAPI_START_STOP_UNIT:
	case USB_ATAPI_PREVENT_ALLOW:
	case USB_ATAPI_READ_FORMAT_CAPACITY:
	case USB_ATAPI_READ_CAPACITY:
		USB_PRINTF0("### Non-mounted command demand generating !\n");
		break;
	case USB_ATAPI_READ10:		// Initialized READ CBW TAG
		g_usb_hmsc_cbw.bm_cbw_flags |= 0x80;
		g_usb_hmsc_cbw.bcbwcb_length = 10;	// 10bytes
		break;
	case USB_ATAPI_WRITE10:		// Initialized WRITE CBW TAG
		g_usb_hmsc_cbw.bm_cbw_flags &= 0x7F;
		g_usb_hmsc_cbw.bcbwcb_length = 10;	// 10bytes
		break;
	case USB_ATAPI_SEEK:
	case USB_ATAPI_WRITE_AND_VERIFY:
	case USB_ATAPI_VERIFY10:
	case USB_ATAPI_MODE_SELECT10:
	case USB_ATAPI_MODE_SENSE10:
	default:
		USB_PRINTF0("### Non-mounted command demand generating !\n");
		break;
	}
	if( USB_ATAPI == g_usb_hmsc_sub_class )			// 12bytes
		g_usb_hmsc_cbw.bcbwcb_length = USB_MSC_CBWCB_LENGTH;
}

/******************************************************************************
 Function Name   : usb_hmsc_set_els_cbw
 Description     : CBW parameter initialization for other commands.
 Arguments       : uint8_t  *cbwcb	: Pointer to the CBW area
                 : uint32_t  trans_byte	: Transfer size
 Return value    : none
 ******************************************************************************/
void usb_hmsc_set_els_cbw(uint8_t *cbwcb, uint32_t trans_byte)
{
uint8_t i;
	// CBW parameter set
	g_usb_hmsc_cbw.dcbw_tag    = usb_hmsc_csw_tag_no;
	g_usb_hmsc_cbw.dcbw_dtl_lo = trans_byte;
	g_usb_hmsc_cbw.dcbw_dtl_ml = trans_byte >> 8;
	g_usb_hmsc_cbw.dcbw_dtl_mh = trans_byte >> 16;
	g_usb_hmsc_cbw.dcbw_dtl_hi = trans_byte >> 24;
	g_usb_hmsc_cbw.bm_cbw_flags = 0;
	g_usb_hmsc_cbw.bcbw_lun = 0;
	g_usb_hmsc_cbw.bcbwcb_length = 0;
	for( i=0 ; i<12 ; i++ )
		g_usb_hmsc_cbw.cbwcb[i] = usb_cbwcb[i];
	// ATAPI command check
	switch( usb_cbwcb[0] )  {
	case USB_ATAPI_TEST_UNIT_READY:		// No data
		g_usb_hmsc_cbw.bcbwcb_length = 6;
		break;
	case USB_ATAPI_REQUEST_SENSE:		// Receive
		g_usb_hmsc_cbw.bm_cbw_flags |= 0x80;
		g_usb_hmsc_cbw.bcbwcb_length = 6;
		break;
	case USB_ATAPI_FORMAT_UNIT:		// Send
		USB_PRINTF0("### Non-mounted command demand generating !\n");
		break;
	case USB_ATAPI_INQUIRY:			// Receive
		g_usb_hmsc_cbw.bm_cbw_flags |= 0x80;
		g_usb_hmsc_cbw.bcbwcb_length = 6;
		break;
	case USB_ATAPI_MODE_SELECT6:
	case USB_ATAPI_MODE_SENSE6:
		break;
	case USB_ATAPI_START_STOP_UNIT:		// No data
		g_usb_hmsc_cbw.bcbwcb_length = 6;
		break;
	case USB_ATAPI_PREVENT_ALLOW:		// No data
		g_usb_hmsc_cbw.bcbwcb_length = 6;
		break;
	case USB_ATAPI_READ_FORMAT_CAPACITY:	// Receive
		g_usb_hmsc_cbw.bm_cbw_flags |= 0x80;
		g_usb_hmsc_cbw.bcbwcb_length = 10;
		break;
	case USB_ATAPI_READ_CAPACITY:		// Receive
		g_usb_hmsc_cbw.bm_cbw_flags |= 0x80;
		g_usb_hmsc_cbw.bcbwcb_length = 10;
		break;
	case USB_ATAPI_READ10:
	case USB_ATAPI_WRITE10:
		USB_PRINTF0("### Non-mounted command demand generating !\n");
		break;
	case USB_ATAPI_SEEK:
	case USB_ATAPI_WRITE_AND_VERIFY:
	case USB_ATAPI_VERIFY10:
		USB_PRINTF0("### Non-mounted command demand generating !\n");
		break;
	case USB_ATAPI_MODE_SELECT10:		// Send
		USB_PRINTF0("### Non-mounted command demand generating !\n");
		break;
	case USB_ATAPI_MODE_SENSE10:		// Receive
		g_usb_hmsc_cbw.bm_cbw_flags |= 0x80;
		g_usb_hmsc_cbw.bcbwcb_length = 10;
		break;
	default:
		USB_PRINTF0("### Non-mounted command demand generating !\n");
		break;
	}
	if( USB_ATAPI == g_usb_hmsc_sub_class )		// 12bytes
		g_usb_hmsc_cbw.bcbwcb_length = USB_MSC_CBWCB_LENGTH;
}

/******************************************************************************
 Function Name   : usb_hmsc_clr_data
 Description     : 12 byte data clear.
 Arguments       : uint8_t *buf		: Pointer to the area to clear
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_clr_data(uint8_t *buf)
{
 uint16_t i;
	for( i=0 ; i<12 ; i++ )
		*buf++ = 0x00;
}

/******************************************************************************
 Function Name   : usb_hmsc_no_data
 Description     : HMSC No data.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_no_data(void)
{
	usb_hmsc_process = USB_MSG_HMSC_NO_DATA;
	usb_hmsc_specified_path( USB_DEVICE0, 0 );
	usb_hmsc_data_seq = USB_SEQ_0;
}

/******************************************************************************
 Function Name   : usb_hmsc_data_in
 Description     : HMSC Data In.
 Arguments       : uint8_t  *buf	: Pointer to the buffer area
                 : uint32_t  size	: Data size
 Return value    : none
 ******************************************************************************/
void usb_hmsc_data_in(uint8_t *buf, uint32_t size)
{
	usb_hmsc_buf = buf;
	usb_hmsc_trans_size = size;
	usb_hmsc_process = USB_MSG_HMSC_DATA_IN;
	usb_hmsc_specified_path( USB_DEVICE0, 0 );
	usb_hmsc_data_seq = USB_SEQ_0;
}

/******************************************************************************
 Function Name   : usb_hmsc_data_out
 Description     : HMSC Data Out.
 Arguments       : uint8_t  *buf	: Pointer to the buffer area
                 : uint32_t  size	: Data size
 Return value    : none
 ******************************************************************************/
void usb_hmsc_data_out(uint8_t *buf, uint32_t size)
{
	usb_hmsc_buf = buf;
	usb_hmsc_trans_size = size;
	usb_hmsc_process = USB_MSG_HMSC_DATA_OUT;
	usb_hmsc_specified_path( USB_DEVICE0, 0 );
	usb_hmsc_data_seq = USB_SEQ_0;
}

/******************************************************************************
 Function Name   : usb_hmsc_data_act
 Description     : Send Data request.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
 Return value    : uint16_t		: 
 ******************************************************************************/
static uint16_t usb_hmsc_data_act(usb_utr_t *msg)
{
uint16_t  retval;
uint16_t  result;
uint8_t  *buf;
uint32_t  size;
	buf = usb_hmsc_buf;
	size = usb_hmsc_trans_size;
	result = msg->result;
	switch( usb_hmsc_data_seq )  {
	case USB_SEQ_0:			// CBW
		retval = usb_hmsc_send_cbw( msg );
		usb_hmsc_data_seq++;
		break;
	case USB_SEQ_1:			// Check CBW and Send Data
		retval = usb_hmsc_send_cbw_check( result);
		if( USB_HMSC_STALL == retval )  {
			usb_hmsc_process = USB_MSG_HMSC_CBW_ERR;
			usb_hmsc_stall_err_seq = USB_SEQ_0;
			usb_hmsc_specified_path( g_usb_hstd_device_addr, 0 );
			usb_hmsc_data_seq = USB_SEQ_0;
		}
		else if( USB_HMSC_OK != retval )  {
			USB_PRINTF0("### Data : SendCBW error \n");
			usb_hmsc_data_seq = USB_SEQ_0;
			usb_hmsc_command_result( msg, retval );
		}
		else
			switch( usb_hmsc_process )  {
			case USB_MSG_HMSC_NO_DATA:
				retval = usb_hmsc_get_csw( );
				usb_hmsc_data_seq = USB_SEQ_4;
				break;
			case USB_MSG_HMSC_DATA_OUT:
				retval = usb_hmsc_send_data( buf, size );
				usb_hmsc_data_seq++;
				break;
			case USB_MSG_HMSC_DATA_IN:
				usb_hmsc_cmd_data_length = 0;
				retval = usb_hmsc_get_data( buf, size );
				usb_hmsc_data_seq++;
				break;
			}
		break;
	case USB_SEQ_2:			// Check Data and Send CSW
		if( USB_MSG_HMSC_DATA_OUT == usb_hmsc_process )  {
			retval = usb_hmsc_send_data_check( result );
			if( USB_HMSC_STALL == retval )  {
				usb_hmsc_clear_stall( usb_hmsc_check_result );
				usb_hmsc_data_seq++;
			}
			else if( USB_HMSC_OK != retval )  {
				USB_PRINTF0("### Data : SendData error \n");
				usb_hmsc_command_result( msg, retval );
				usb_hmsc_data_seq = USB_SEQ_0;
			}
			else  {
				retval = usb_hmsc_get_csw( );
				usb_hmsc_data_seq = USB_SEQ_4;
			}
		}
		else if( USB_MSG_HMSC_DATA_IN == usb_hmsc_process )  {
			retval = usb_hmsc_get_data_check( result );
			if( USB_HMSC_STALL == retval )  {
				usb_hmsc_clear_stall( usb_hmsc_check_result );
				usb_hmsc_data_seq++;
			}
			else if( USB_HMSC_OK != retval )  {
				USB_PRINTF0("### Data : SendData error \n");
				usb_hmsc_command_result(msg, retval);
				usb_hmsc_data_seq = USB_SEQ_0;
			}
			else  {
				usb_hmsc_cmd_data_length = msg->tranlen;
				retval = usb_hmsc_get_csw( );
				usb_hmsc_data_seq = USB_SEQ_4;
			}
		}
		break;
	case USB_SEQ_3:			// Check ClearStall and Send CSW
		retval = usb_hmsc_get_csw( );
		usb_hmsc_data_seq++;
		break;
	case USB_SEQ_4:			// Check CSW
		usb_hmsc_data_seq = USB_SEQ_0;
		retval = usb_hmsc_get_csw_check( msg, result );
		switch( retval )  {
		case USB_HMSC_OK:		// Success
			if( USB_ON == usb_hmsc_csw_err_loop )  {
				usb_hmsc_csw_err_loop = USB_OFF;
				retval = USB_HMSC_CSW_ERR;
			}
			usb_hmsc_command_result( msg, retval );
			break;
		case USB_HMSC_CSW_ERR:
			USB_PRINTF0("*** Data : CSW-NG \n");
			if( USB_MSG_HMSC_STRG_USER_COMMAND != g_usb_hmsc_strg_process )  {
				usb_hmsc_csw_err_loop = USB_ON;
				usb_hmsc_request_sense( buf );
			}
			else  {
				if( USB_ON == usb_hmsc_csw_err_loop )
					usb_hmsc_csw_err_loop = USB_OFF;
				usb_hmsc_command_result( msg, retval );
			}
			break;
		case USB_HMSC_STALL:
			USB_PRINTF0("*** Data : CSW-STALL \n");
			g_usb_hmsc_in_pipectr = 0;
			usb_hmsc_clear_stall( usb_hmsc_check_result );
			usb_hmsc_data_seq = USB_SEQ_3;
			break;
		case USB_HMSC_CSW_PHASE_ERR:
			USB_PRINTF0("*** Data : CSW-PhaseError \n");
			usb_hmsc_process = USB_MSG_HMSC_CSW_PHASE_ERR;
			usb_hmsc_stall_err_seq = USB_SEQ_0;
			usb_hmsc_specified_path( g_usb_hstd_device_addr, 0 );
			break;
		}
		break;
	}
	return retval;
}

/******************************************************************************
 Function Name   : usb_hmsc_stall_err
 Description     : HMSC Stall Error.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_stall_err(usb_utr_t *msg)
{
static uint16_t devadr;
uint16_t result;
	switch( usb_hmsc_stall_err_seq )  {
	case USB_SEQ_0:
		devadr = msg->keyword;				// Control Transfer overlaps
		if( USB_QOVR == usb_hmsc_mass_storage_reset( devadr, usb_hmsc_check_result ) )
			usb_hmsc_message_retry( USB_HMSC_MBX, msg );
		else						// Control Transfer not overlaps
			usb_hmsc_stall_err_seq++;
		break;
	case USB_SEQ_1:
		if( USB_OK != usb_hmsc_ref_drvno( devadr ) )  {
			usb_hmsc_command_result( msg, USB_HMSC_CBW_ERR );
			usb_hmsc_stall_err_seq = USB_SEQ_0;
			return;
		}
		usb_hmsc_clear_stall( usb_hmsc_check_result );
		usb_hmsc_stall_err_seq++;
		g_usb_hmsc_out_pipectr = 0;
		break;
	case USB_SEQ_2:
		if( USB_OK != usb_hmsc_ref_drvno( devadr ) )  {
			usb_hmsc_command_result( msg, USB_HMSC_CBW_ERR );
			usb_hmsc_stall_err_seq = USB_SEQ_0;
			return;
		}
		usb_hmsc_clear_stall( usb_hmsc_check_result );
		usb_hmsc_stall_err_seq++;
		g_usb_hmsc_in_pipectr = 0;
		break;
	case USB_SEQ_3:
		if( USB_MSG_HMSC_CSW_PHASE_ERR == msg->msginfo )
			result = USB_HMSC_CSW_PHASE_ERR;
		else
			result = USB_HMSC_CBW_ERR;
		usb_hmsc_command_result( msg, result);
		usb_hmsc_stall_err_seq = USB_SEQ_0;
		break;
	default:
		if( USB_MSG_HMSC_CSW_PHASE_ERR == msg->msginfo )
			result = USB_HMSC_CSW_PHASE_ERR;
		else
			result = USB_HMSC_CBW_ERR;
		usb_hmsc_command_result( msg, result );
		usb_hmsc_stall_err_seq = USB_SEQ_0;
		break;
	}
}

/******************************************************************************
 Function Name   : usb_hmsc_specified_path
 Description     : Next Process Selector.
 Arguments       : uint16_t keyword	: Root port/Device address/Pipe number
                 : uint16_t result	: Result
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_specified_path(uint16_t keyword, uint16_t result)
{
usb_utr_t *msg;
	msg = USB_GET_BLK( );			// Get Message Block
	msg->msginfo = usb_hmsc_process;
        msg->keyword = keyword;
        msg->result  = result;
	USB_SND_MSG( USB_HMSC_MBX, msg );	// Send Message
}

/******************************************************************************
 Function Name   : usb_hmsc_check_result
 Description     : Hub class check result.
 Arguments       : usb_utr_t  *msg	: Pointer to usb_utr_t structure
                 : uint16_t   data1	: Not used
                 : uint16_t   data2	: Not used
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_check_result(usb_utr_t *msg, uint16_t data1, uint16_t data2)
{
	usb_hmsc_specified_path( 0, msg->status );
}

/******************************************************************************
 Function Name   : usb_hmsc_command_result
 Description     : Hub class check result.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
                 : uint16_t  result	: Command Result
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_command_result(usb_utr_t *msg, uint16_t result)
{
usb_utr_t *blk;
	blk = USB_GET_BLK( );			// Get Message Block
	blk->msginfo = g_usb_hmsc_strg_process;
	blk->result = result;
	blk->tranlen = msg->tranlen;
	USB_SND_MSG( USB_HSTRG_MBX, blk );	// Send message
}

/******************************************************************************
 Function Name   : usb_hmsc_get_string_desc
 Description     : Set GetDescriptor.
 Arguments       : uint16_t string	: String Descriptor index
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_get_string_desc(uint16_t string)
{
usb_utr_t *msg;
	if( ! string )  {
		trans_table.setup.index  = 0x0000;
		trans_table.setup.length = 0x0004;
	}
	else  {						// Set LanguageID
		trans_table.setup.index  = g_usb_buf[2];
		trans_table.setup.index |= g_usb_buf[3] << 8;
		trans_table.setup.length = USB_HMSC_CLSDATASIZE;
	}
	trans_table.setup.type  = USB_GET_DESCRIPTOR | USB_DEV_TO_HOST | USB_STANDARD | USB_DEVICE;
	trans_table.setup.value = USB_STRING_DESCRIPTOR + string;
	trans_table.address = USB_DEVICE1;
	msg = USB_GET_BLK( );				// Get Message Block
	msg->tranadr  = g_usb_buf;
	msg->complete = usb_hmsc_class_check_result;
	msg->tranlen  = trans_table.setup.length;
	msg->keyword  = USB_PIPE0;
	msg->setup    = &trans_table.setup.type;
	msg->segment  = USB_TRAN_END;
	usb_hstd_transfer_start_req( msg );
}

/******************************************************************************
 Function Name   : usb_hmsc_send_cbw
 Description     : Send CBW.
 Arguments       : usb_utr_t  *msg	: Pointer to usb_utr_t structure
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_send_cbw(usb_utr_t *msg)
{
uint16_t toggle;
usb_er_t retval;
	usb_hmsc_csw_tag_no++;				// Set CBW TAG usb_hmsc_CbwTagCoun
	if( ! usb_hmsc_csw_tag_no )
		usb_hmsc_csw_tag_no = 1;
	usb_hmsc_trans_data.keyword  = USB_PIPE1;		// pipe number
	usb_hmsc_trans_data.tranadr  = &g_usb_hmsc_cbw;		// Transfer data address
	usb_hmsc_trans_data.tranlen  = USB_MSC_CBWLENGTH;	// Transfer data length
	usb_hmsc_trans_data.complete = usb_hmsc_check_result;	// CallBack Function Info
	usb_hmsc_trans_data.setup    = 0;			// Not control transfer
	usb_hmsc_trans_data.segment  = USB_TRAN_END;
	usb_hstd_set_pipe_info( USB_PIPE1, &usb_hmsc_pipe_table[USB_PIPE_DIR_OUT] );
	usb_hstd_set_pipe_reg( );
	if( USB_SQMON == ( USB_SQMON & g_usb_hmsc_out_pipectr ) )
		toggle = USB_DO_SET_SQTGL;
	else
		toggle = USB_DO_CLR_SQTGL;
	usb_hstd_change_device_state( usb_hstd_dummy_function, toggle );
	retval = usb_hstd_transfer_start( &usb_hmsc_trans_data );
	if( USB_OK != retval )  {
		USB_PRINTF0("### Mass Storage Device Class submit error !\n");
		return USB_HMSC_SUBMIT_ERR;
	}
	return retval;
}

/******************************************************************************
 Function Name   : usb_hmsc_send_cbw_check
 Description     : Check send CBW.
 Arguments       : uint16_t result	: Transfer Result
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_send_cbw_check(uint16_t result)
{
	switch( result )  {
	case USB_DATA_NONE:	// Send CBW
		g_usb_hmsc_out_pipectr = usb_hmsc_trans_data.pipectr;
		return USB_HMSC_OK;
	case USB_DATA_STALL:	// Stall
		USB_PRINTF0("*** CBW Transfer STALL !\n");
		return USB_HMSC_STALL;
	case USB_DATA_ERR:
		USB_PRINTF0("### CBW Transfer ERROR !\n");
		break;
	default:
		USB_PRINTF1("### CBW Transfer error result:%d !\n", result);
		break;
	}
	return USB_HMSC_CBW_ERR;
}

/******************************************************************************
 Function Name   : usb_hmsc_get_data
 Description     : Receive Data request.
 Arguments       : uint8_t  *buf	: Pointer to the area to store the data
                 : uint32_t size	: Receive Data Size
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_get_data(uint8_t *buf, uint32_t size)
{
uint16_t toggle;
usb_er_t retval;
	usb_hmsc_receive_data.keyword  = USB_PIPE1;		// pipe number
	usb_hmsc_receive_data.tranadr  = buf;			// Transfer data address
	usb_hmsc_receive_data.tranlen  = size;			// Transfer data length
	usb_hmsc_receive_data.complete = usb_hmsc_check_result;	// CallBack Function Info
	usb_hmsc_receive_data.setup    = 0;			// Not control transfer
	usb_hmsc_receive_data.segment  = USB_TRAN_END;
	usb_hstd_set_pipe_info( USB_PIPE1, &usb_hmsc_pipe_table[USB_PIPE_DIR_IN] );
	usb_hstd_set_pipe_reg( );
	if( USB_SQMON == ( USB_SQMON & g_usb_hmsc_in_pipectr ) )
		toggle = USB_DO_SET_SQTGL;
	else
		toggle = USB_DO_CLR_SQTGL;
	usb_hstd_change_device_state( usb_hstd_dummy_function, toggle );
	retval = usb_hstd_transfer_start( &usb_hmsc_receive_data );
	if( USB_OK != retval )  {
		USB_PRINTF0("### Mass Storage Device Class submit error !\n");
		return USB_HMSC_SUBMIT_ERR;
	}
	return retval;
}

/******************************************************************************
 Function Name   : usb_hmsc_get_data_check
 Description     : Check Get Data .
 Arguments       : uint16_t result	: Transfer Result
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_get_data_check(uint16_t result)
{
	switch( result )  {
	case USB_DATA_SHT:
	case USB_DATA_OK:	// Continue
		g_usb_hmsc_in_pipectr = usb_hmsc_receive_data.pipectr;
		return USB_HMSC_OK;
	case USB_DATA_STALL:
		USB_PRINTF0("*** GetData STALL !\n");
		g_usb_hmsc_in_pipectr = 0;
		return USB_HMSC_STALL;
	case USB_DATA_ERR:
		USB_PRINTF0("### GetData ERROR !\n");
		break;
	case USB_DATA_OVR:
		USB_PRINTF0("### GetData over !\n");
		break;
	default:
		USB_PRINTF0("### GetData error !\n");
		break;
	}
	return USB_HMSC_DAT_RD_ERR;
}

/******************************************************************************
 Function Name   : usb_hmsc_send_data
 Description     : Send Pipe Data.
 Arguments       : uint8_t   *buff	: Pointer to the area to store the data
                 : uint32_t  size	: Receive Data Size
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_send_data(uint8_t *buf, uint32_t size)
{
uint16_t toggle;
usb_er_t retval;
	usb_hmsc_trans_data.keyword  = USB_PIPE1;		// pipe number
	usb_hmsc_trans_data.tranadr  = buf;			// Transfer data address
	usb_hmsc_trans_data.tranlen  = size;			// Transfer data length
	usb_hmsc_trans_data.complete = usb_hmsc_check_result;	// CallBack Function Info
	usb_hmsc_trans_data.setup    = 0;			// Not control transfer
	usb_hmsc_trans_data.segment  = USB_TRAN_END;
	usb_hstd_set_pipe_info( USB_PIPE1, &usb_hmsc_pipe_table[USB_PIPE_DIR_OUT] );
	usb_hstd_set_pipe_reg( );
	if( USB_SQMON == ( USB_SQMON & g_usb_hmsc_out_pipectr ) )
		toggle = USB_DO_SET_SQTGL;
	else
		toggle = USB_DO_CLR_SQTGL;
	usb_hstd_change_device_state( usb_hstd_dummy_function, toggle );
	retval = usb_hstd_transfer_start( &usb_hmsc_trans_data );
	if( USB_OK != retval )  {
		USB_PRINTF0("### Mass Storage Device Class submit error !\n");
		return USB_HMSC_SUBMIT_ERR;
	}
	return retval;
}

/******************************************************************************
 Function Name   : usb_hmsc_send_data_check
 Description     : Check Send Data.
 Arguments       : uint16_t result	: Transfer Result
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_send_data_check(uint16_t result)
{
	switch( result )  {
	case USB_DATA_NONE:
		g_usb_hmsc_out_pipectr = usb_hmsc_trans_data.pipectr;
		return USB_HMSC_OK;
	case USB_DATA_STALL:
		USB_PRINTF0("*** SendData STALL !\n");
		g_usb_hmsc_out_pipectr = 0;
		return USB_HMSC_STALL;
	case USB_DATA_ERR:
		USB_PRINTF0("### SendData ERROR !\n");
		break;
	default:
		USB_PRINTF0("### SendData error !\n");
		break;
	}
	return USB_HMSC_DAT_WR_ERR;
}

/******************************************************************************
 Function Name   : usb_hmsc_get_csw
 Description     : Receive CSW.
 Arguments       : none
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_get_csw(void)
{
uint16_t toggle;
usb_er_t retval;
	usb_hmsc_receive_data.keyword  = USB_PIPE1;		// pipe number
	usb_hmsc_receive_data.tranadr  = &g_usb_hmsc_csw;	// Transfer data address
	usb_hmsc_receive_data.tranlen  = USB_MSC_CSW_LENGTH;	// Transfer data length
	usb_hmsc_receive_data.complete = usb_hmsc_check_result;	// CallBack Function Info
	usb_hmsc_receive_data.setup    = 0;
	usb_hmsc_receive_data.segment  = USB_TRAN_END;		// Not control transfer
	usb_hstd_set_pipe_info( USB_PIPE1, &usb_hmsc_pipe_table[USB_PIPE_DIR_IN] );
	usb_hstd_set_pipe_reg( );
	if( USB_SQMON == ( USB_SQMON & g_usb_hmsc_in_pipectr ) )
		toggle = USB_DO_SET_SQTGL;
	else
		toggle = USB_DO_CLR_SQTGL;
	usb_hstd_change_device_state( usb_hstd_dummy_function, toggle );
	retval = usb_hstd_transfer_start( &usb_hmsc_receive_data );
	if( USB_OK != retval )  {
		USB_PRINTF0("### Mass Storage Device Class submit error !\n");
		return USB_HMSC_SUBMIT_ERR;
	}
	return retval;
}

/******************************************************************************
 Function Name   : usb_hmsc_get_csw_check
 Description     : Check Receive CSW.
 Arguments       : usb_utr_t  *msg	: Pointer to usb_utr_t structure
                 : uint16_t   result	: Transfer Result
 Return value    : uint16_t		: Error Code
 ******************************************************************************/
static uint16_t usb_hmsc_get_csw_check(usb_utr_t *msg, uint16_t result)
{
uint32_t request_size;
	msg->tranlen = 0;
	msg->status = g_usb_hmsc_csw.dcsw_status;
	switch( result )  {
	case USB_DATA_SHT:
	case USB_DATA_OK:		// Continue
		g_usb_hmsc_in_pipectr = usb_hmsc_receive_data.pipectr;
		request_size  = g_usb_hmsc_cbw.dcbw_dtl_lo;
		request_size |= g_usb_hmsc_cbw.dcbw_dtl_ml << 8;
		request_size |= g_usb_hmsc_cbw.dcbw_dtl_mh << 16;
		request_size |= g_usb_hmsc_cbw.dcbw_dtl_hi << 24;
		msg->tranlen = request_size - usb_hmsc_cmd_data_length;
		if( USB_MSC_CSW_SIGNATURE != g_usb_hmsc_csw.dcsw_signature )  {	// CSW Check
			USB_PRINTF2("### CSW signature error 0x%08x:SIGN=0x%08x.\n",
			g_usb_hmsc_csw.dcsw_signature, USB_MSC_CSW_SIGNATURE);
			return USB_HMSC_CSW_ERR;
		}
		if( g_usb_hmsc_csw.dcsw_tag != g_usb_hmsc_cbw.dcbw_tag )  {
			USB_PRINTF2("### CSW Tag error 0x%08x:CBWTAG=0x%08x.\n",
                        g_usb_hmsc_csw.dcsw_tag, g_usb_hmsc_cbw.dcbw_tag);
			return USB_HMSC_CSW_ERR;
		}
		switch( g_usb_hmsc_csw.dcsw_status )  {
		case USB_MSC_CSW_OK:
			return USB_HMSC_OK;
		case USB_MSC_CSW_NG:
			return USB_HMSC_CSW_ERR;
		case USB_MSC_CSW_PHASE_ERR:
			return USB_HMSC_CSW_PHASE_ERR;
		}
		return USB_HMSC_CSW_ERR;
		break;
	case USB_DATA_STALL:		// Stall
		USB_PRINTF0("*** GetCSW Transfer STALL !\n");
		return USB_HMSC_STALL;
	case USB_DATA_ERR:
		USB_PRINTF0("### GetCSW Transfer ERROR !\n");
		break;
	case USB_DATA_OVR:
		USB_PRINTF0("### GetCSW receive over !\n");
		break;
	default:
		USB_PRINTF0("### GetCSW Transfer error !\n");
		break;
	}
	return USB_HMSC_CSW_ERR;
}

/******************************************************************************
 Function Name   : usb_hmsc_clear_stall
 Description     : Clear Stall.
 Arguments       : usb_cb_t complete	: Callback function
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_clear_stall(usb_cb_t complete)
{
	usb_hstd_change_device_state( complete, USB_DO_CLR_STALL );
}

/******************************************************************************
 Function Name   : usb_hmsc_std_req_check
 Description     : Sample Standard Request Check.
 Arguments       : uint16_t result	: Transfer Result
 Return value    : uint16_t		: error info
 ******************************************************************************/
static uint16_t usb_hmsc_std_req_check(uint16_t result)
{
	if( USB_DATA_STALL == result )  {
		USB_PRINTF0("*** Standard Request STALL !\n");
		return USB_ERROR;
	}
	else if( USB_CTRL_END != result )  {
		USB_PRINTF0("*** Standard Request error !\n");
		return USB_ERROR;
	}
	return USB_OK;
}

/******************************************************************************
 Function Name   : usb_hmsc_class_check_result
 Description     : Hub class check result.
 Arguments       : usb_utr_t  *msg	: Pointer to usb_utr_t structure
                 : uint16_t   data1	: Not used
                 : uint16_t   data2	: Not used
 Return value    : none
 ******************************************************************************/
static void usb_hmsc_class_check_result(usb_utr_t *msg, uint16_t data1, uint16_t data2)
{
usb_utr_t *blk;
	blk = USB_GET_BLK( );			// Get Message Block
	blk->msginfo = USB_MSG_CLS_INIT;
	blk->keyword = msg->keyword;
	blk->result  = msg->status;
	USB_SND_MSG( USB_HMSC_MBX, blk );	// Send Message
}

/******************************************************************************
 Function Name   : usb_hmsc_pipe_info
 Description     : Pipe Information.
 Arguments       : uint8_t  *table	: Pointer to the pipe information table
                 : uint16_t  speed	: USB speed
                 : uint16_t  length	: Data size
 Return value    : uint16_t		: 
 ******************************************************************************/
uint16_t usb_hmsc_pipe_info(uint8_t *table, uint16_t speed, uint16_t length)
{
uint8_t	out_pipe = USB_NULL;
uint8_t	 in_pipe = USB_NULL;
uint16_t epcnt = 0;
uint16_t ofdsc;
uint16_t devadr;
uint8_t *pdescriptor;
	if( USB_DT_INTERFACE != table[1] )  {		// Check Descriptor
		USB_PRINTF0("### Not Interface pdescriptor.\n"); /* Configuration Descriptor */
		return USB_ERROR;
	}
	ofdsc = table[0];				// Check Endpoint Descriptor
	while( ofdsc < length - table[0] )  {
		if( USB_DT_ENDPOINT == table[ofdsc+1] )		// Search within Interface
			if( USB_EP_BULK == ( table[ofdsc+3] & USB_EP_TRNSMASK ) )  {	// Bulk Endpoint
				pdescriptor = &table[ofdsc];
				devadr = g_usb_hstd_device_addr;		// Get USB Device address
				if( USB_EP_IN == ( pdescriptor[2] & USB_EP_DIRMASK ) )
					in_pipe = usb_hstd_make_pipe_reg_info( devadr, pdescriptor, &usb_hmsc_pipe_table[USB_PIPE_DIR_IN] );
				else
					out_pipe = usb_hstd_make_pipe_reg_info( devadr, pdescriptor, &usb_hmsc_pipe_table[USB_PIPE_DIR_OUT] );
				if( USB_NULL != in_pipe && USB_NULL != out_pipe )
					return USB_OK;
				epcnt++;
				if( epcnt >= 2 )  {
					USB_PRINTF0("### Endpoint Descriptor error.\n");
					return USB_ERROR;
				}
			}
		ofdsc += table[ofdsc];
	}
	return USB_ERROR;
}

/******************************************************************************
 Function Name   : usb_hmsc_configured
 Description     : Callback function for MSC device configured.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hmsc_configured(void)
{
	if( USB_ERROR == usb_hmsc_strg_drive_open( USB_DEVICE1 ) )  {
		USB_PRINTF0("### usb_hmsc_strg_drive_open error\n");
		USB_NoSupportEvent( );			// Set USB Event(No Support)
	}
}

/******************************************************************************
 Function Name   : usb_hmsc_detach
 Description     : Callback function for MSC device detach.
 Arguments       : uint16_t devadr	: Device Address
 Return value    : none
 ******************************************************************************/
void usb_hmsc_detach(uint16_t devadr)
{
	usb_hstd_clr_pipe_table( devadr);		// Clear pipe table
	if( g_drive_search_lock == devadr )
		g_drive_search_lock = USB_FALSE;
	if( g_drive_search_que_cnt && g_drive_search_que == devadr )  {
		g_drive_search_que = USB_FALSE;
		g_drive_search_que_cnt--;
	}
	if( USB_OK == usb_hmsc_ref_drvno( devadr ) )
		usb_hmsc_strg_drive_close( );		// Callback when device is detached
	USB_DetachEvent( );				// Set USB Event(USB Detach)
}

/******************************************************************************
 Function Name   : usb_hmsc_drive_complete
 Description     : Next Process Selector.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
                 : uint16_t  devadr	: Device Address
                 : uint16_t  data2	: Not used
 Return          : none
 ******************************************************************************/
void usb_hmsc_drive_complete(usb_utr_t *msg, uint16_t devadr, uint16_t data2)
{
	if( g_drive_search_que_cnt )
		g_drive_search_que_cnt--;
	g_drive_search_lock = USB_FALSE;
	USB_AttachEvent( );				// Set USB Event(USB Attach)
}

/******************************************************************************
 Function Name   : usb_hmsc_message_retry
 Description     : Message transfer retry for Que Over.
 Arguments       : uint16_t   id	: Mail Box ID
                 : usb_utr_t *msg	: Pointer to usb_utr_t structure
 Return value    : none
 ******************************************************************************/
void usb_hmsc_message_retry(uint16_t id, usb_utr_t *msg)
{
usb_utr_t *blk;
	if( USB_HSTRG_MBX == id )  {		// Resend message
		blk = USB_GET_BLK( );
		blk->msginfo  = msg->msginfo;
		blk->keyword  = msg->keyword;
		blk->result   = msg->result;
		blk->complete = msg->complete;
	        USB_SND_MSG( id, blk );		// Send message
	}
}

/******************************************************************************
 Function Name   : usb_hmsc_driver_start
 Description     : USB Host Initialize process.
 Argument        : none
 Return          : none
 ******************************************************************************/
void usb_hmsc_driver_start(void)
{
	usb_hmsc_csw_tag_no = 1;
	g_usb_hmsc_cbw.dcbw_signature = USB_MSC_CBW_SIGNATURE;
	g_usb_hmsc_cbw.dcbw_tag = usb_hmsc_csw_tag_no;
	usb_cstd_set_task_pri( USB_HMSC_TSK,  USB_PRI_3 );
	usb_cstd_set_task_pri( USB_HSTRG_TSK, USB_PRI_3 );
}

/******************************************************************************
 Function Name   : usb_hmsc_class_check
 Description     : check connected device.
 Arguments       : uint16_t **table	: Pointer to the class information table
 Return value    : none
 ******************************************************************************/
void usb_hmsc_class_check(uint16_t **table)
{
usb_utr_t *msg;
	usb_hmsc_device_table = (uint8_t*)table[0];
	g_usb_hmsc_config_table = (uint8_t*)table[1];
	g_usb_hmsc_interface_table = (uint8_t*)table[2];
	g_usb_hmsc_speed = *table[6];
	*table[3] = USB_OK;
	msg = USB_GET_BLK( );			// Get Message Block
	msg->msginfo = USB_MSG_CLS_INIT;
	usb_hmsc_init_seq = USB_SEQ_0;
	USB_SND_MSG( USB_HMSC_MBX, msg );	// Send Message
}

/******************************************************************************
 Function Name   : usb_hmsc_read10
 Description     : Read10.
 Arguments       : uint8_t  *buf	: Buffer address
                 : uint32_t  secno	: Sector number
                 : uint16_t  seccnt	: Sector count
                 : uint32_t  trans_byte	: Trans byte
 Return value    : none
 ******************************************************************************/
void usb_hmsc_read10(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte)
{							// set CBW parameter
	usb_hmsc_set_rw_cbw( USB_ATAPI_READ10, secno, seccnt, trans_byte );
	usb_hmsc_data_in( buf, trans_byte );		// Data IN
}

/******************************************************************************
 Function Name   : usb_hmsc_write10
 Description     : Write10.
 Arguments       : uint8_t  *buf	: Buffer address
                 : uint32_t  secno	: Sector number
                 : uint16_t  seccnt	: Sector count
                 : uint32_t  trans_byte	: Trans byte
 Return value    : none 
 ******************************************************************************/
void usb_hmsc_write10(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte)
{							// set CBW parameter
	usb_hmsc_set_rw_cbw( USB_ATAPI_WRITE10, secno, seccnt, trans_byte );
	usb_hmsc_data_out( buf, trans_byte );		// Data OUT
}

/******************************************************************************
 Function Name   : usb_hmsc_test_unit
 Description     : TestUnit.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hmsc_test_unit(void)
{
	usb_hmsc_clr_data( usb_cbwcb );			// Data clear
	usb_cbwcb[0] = USB_ATAPI_TEST_UNIT_READY;	// Command set
	usb_hmsc_set_els_cbw( usb_cbwcb, 0 );		// Set CBW parameter
	usb_hmsc_no_data( );				// No Data
}

/******************************************************************************
 Function Name   : usb_hmsc_request_sense
 Description     : RequestSense.
 Arguments       : uint8_t *buf		: Pointer to the buffer area
 Return value    : none
 ******************************************************************************/
void usb_hmsc_request_sense(uint8_t *buf)
{
uint8_t length = 18;
	usb_hmsc_clr_data( usb_cbwcb );				// Data clear
	usb_cbwcb[0] = USB_ATAPI_REQUEST_SENSE;			// Command set
	usb_cbwcb[4] = length;					// Allocation length
	usb_hmsc_set_els_cbw( usb_cbwcb, length );		// Set CBW parameter
	usb_hmsc_data_in( buf, length );			// Data IN
}

/******************************************************************************
 Function Name   : usb_hmsc_inquiry
 Description     : Inquiry.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hmsc_inquiry(void)
{
uint8_t length = 36;
	usb_hmsc_clr_data( usb_cbwcb );			// Data clear
	usb_cbwcb[0] = USB_ATAPI_INQUIRY;		// Command set
	usb_cbwcb[4] = length;				// Allocation length
	usb_hmsc_set_els_cbw( usb_cbwcb, length );	// Set CBW parameter
	usb_hmsc_data_in( g_usb_buf, length );		// Data IN
}

/******************************************************************************
 Function Name   : usb_hmsc_read_capacity
 Description     : ReadCapacity.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hmsc_read_capacity(void)
{
uint8_t length = 8;
	usb_hmsc_clr_data( usb_cbwcb );			// Data clear
	usb_cbwcb[0] = USB_ATAPI_READ_CAPACITY;		// Command set
	usb_hmsc_set_els_cbw( usb_cbwcb, length );	// Set CBW parameter
	usb_hmsc_data_in( g_usb_buf, length );		// Data IN
}

/******************************************************************************
 Function Name   : usb_hmsc_read_format_capacity
 Description     : ReadFormatCapacity.
 Arguments       : none
 Return value    : none
 ******************************************************************************/
void usb_hmsc_read_format_capacity(void)
{
uint8_t length = 0x20;
	usb_hmsc_clr_data( usb_cbwcb );			// Data clear
	usb_cbwcb[0] = USB_ATAPI_READ_FORMAT_CAPACITY;	// Command set
	usb_cbwcb[8] = length;				// Allocation length
	usb_hmsc_set_els_cbw( usb_cbwcb, length );	// Set CBW parameter
	usb_hmsc_data_in( g_usb_buf, length );		// Data IN
}

/******************************************************************************
 Function Name   : usb_hmsc_get_max_unit
 Description     : Get Max LUN request.
 Arguments       : uint16_t  devadr	: Device address
                 : uint8_t  *buf	: Pointer to the buffer area
                 : usb_cb_t  complete	: CallBack Function
 Return value    : usb_er_t		: Error Code
 ******************************************************************************/
usb_er_t usb_hmsc_get_max_unit(uint16_t devadr, uint8_t *buf, usb_cb_t complete)
{
usb_utr_t *msg;
	trans_table.setup.type   = 0xFEA1;
	trans_table.setup.value  = 0x0000;
	trans_table.setup.index  = 0x0000;
	trans_table.setup.length = 0x0001;
	trans_table.address = devadr;
	msg = USB_GET_BLK( );			// Get Message Block
	msg->keyword  = USB_PIPE0;
	msg->tranadr  = buf;
	msg->tranlen  = trans_table.setup.length;
	msg->setup    = &trans_table.setup.type;
	msg->complete = complete;
	msg->segment  = USB_TRAN_END;
	return usb_hstd_transfer_start( msg );
}

/******************************************************************************
 Function Name   : usb_hmsc_mass_storage_reset
 Description     : Mass Strage Reset request.
 Arguments       : uint16_t devadr	: Device address
                 : usb_cb_t complete	: CallBack Function
 Return value    : usb_er_t		: Error Code
 ******************************************************************************/
usb_er_t usb_hmsc_mass_storage_reset(uint16_t devadr, usb_cb_t complete)
{
usb_utr_t *msg;
	trans_table.setup.type   = 0xFF21;
	trans_table.setup.value  = 0x0000;
	trans_table.setup.index  = 0x0000;
	trans_table.setup.length = 0x0000;
	trans_table.address = devadr;
	msg = USB_GET_BLK( );			// Get Message Block
	msg->keyword  = USB_PIPE0;
	msg->tranadr  = USB_NULL;
	msg->tranlen  = trans_table.setup.length;
	msg->setup    = &trans_table.setup.type;
	msg->complete = complete;
	msg->segment  = USB_TRAN_END;
	return usb_hstd_transfer_start( msg );
}

/******************************************************************************
 Function Name   : usb_hmsc_ref_drvno
 Description     : Get Drive no.
 Arguments       : uint16_t devadr	: Device address
 Return value    : USB_OK/USB_ERROR
 ******************************************************************************/
uint16_t usb_hmsc_ref_drvno(uint16_t devadr)
{
	if( devadr == g_usb_hstd_device_addr )
		return USB_OK;
	return USB_ERROR;
}
