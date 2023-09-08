/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hmanager.h
 */

#include "r_usb_typedef.h"
#include "r_usb_extern.h"

static uint8_t  usb_hstd_enum_seq;
static uint16_t usb_hstd_std_request[5];

static uint16_t usb_hstd_chk_device_class(usb_utr_t *msg);
static void     usb_hstd_enumeration_err(uint16_t Rnum);
static void     usb_hstd_enum_set_address(uint16_t data1, uint16_t data2);
static void     usb_hstd_enum_set_configuration(uint16_t confnum, uint16_t data2);
static void     usb_hstd_enum_dummy_request(uint16_t data1, uint16_t data2);
static void     usb_hstd_submit_result(usb_utr_t *msg, uint16_t data1, uint16_t data2);

uint8_t          g_usb_hstd_check_enu_result;
usb_descriptor_t g_usb_hstd_device_descriptor[USB_DEVICESIZE/2];
usb_descriptor_t g_usb_hstd_config_descriptor[USB_CONFIGSIZE/2];

static void (* const g_usb_hstd_enumaration_process[8])(uint16_t, uint16_t) = {
	usb_hstd_enum_get_descriptor, usb_hstd_enum_set_address,
	usb_hstd_enum_get_descriptor, usb_hstd_enum_get_descriptor,
	usb_hstd_enum_get_descriptor, usb_hstd_enum_get_descriptor,
	usb_hstd_enum_set_configuration, usb_hstd_enum_dummy_request,
};							// Enumeration Table

/******************************************************************************
 Function Name   : usb_hstd_enumeration
 Description     : Execute enumeration on the connected USB device.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
 Return          : uint16_t		: Enumeration status.
 ******************************************************************************/
static uint16_t usb_hstd_enumeration(usb_utr_t *msg)
{
uint16_t enume_mode;
uint8_t *descriptor_table;
uint16_t devsel;
uint16_t retval;
	enume_mode = USB_NONDEVICE;			// Attach Detect Mode
	switch( msg->result )  {			// Manager Mode Change
	case USB_CTRL_END:
		enume_mode = USB_DEVICEENUMERATION;
		switch( usb_hstd_enum_seq )  {
		case 0:			// Receive Device Descriptor
		case 2:			// Receive Device Descriptor(18)
		case 3:			// Receive Configuration Descriptor(9)
			break;
		case 1:			// Set Address
			descriptor_table = &g_usb_hstd_device_descriptor[0].byte;
			devsel = g_usb_hstd_device_addr << USB_DEVADDRBIT;
			usb_hstd_set_dev_addr( devsel, g_usb_hstd_device_speed );	// Set device speed
			g_usb_hstd_dcp_register[g_usb_hstd_device_addr] = descriptor_table[7] & USB_MAXPFIELD | devsel;
			break;
                case 4:			// Receive Configuration Descriptor(xx)
			if( USB_DETACHED == g_usb_hstd_device_state )  {
				retval = usb_hstd_chk_device_class( msg );
				g_usb_hstd_check_enu_result = USB_OK;
				// In this function, check device class of enumeration flow move to class
				// "usb_hstd_return_enu_mgr()" is used to return
				if( USB_OK == retval )
					break;
			}
			USB_NoSupportEvent( );					// Set USB Event(No Support)
			usb_hstd_enum_seq++;
			break;

		case 5:			// Class Check Result
			switch( g_usb_hstd_check_enu_result )  {
			case USB_OK:
				break;
			case USB_ERROR:
				enume_mode = USB_NOTTPL;
				break;
			default:
				enume_mode = USB_NONDEVICE;
				break;
			}
			break;
		case 6:			// Set Configuration
			USB_PRINTF0(" Configured Device\n");	// Device enumeration function
			if( g_usb_hstd_device_addr == USB_DEVICE1 )  {
				g_usb_hstd_device_info[1] = USB_CONFIGURED;		// Device state
				g_usb_hstd_device_info[4] = g_usb_hstd_device_speed;	// Device speed
				g_usb_hstd_device_state   = USB_CONFIGURED;		// Device state
				usb_hmsc_configured( );					// Call Back
				return USB_COMPLETEPIPESET;
			}
			enume_mode = USB_COMPLETEPIPESET;
			break;
		}
		usb_hstd_enum_seq++;
		if( USB_DEVICEENUMERATION == enume_mode )  {		// Device Enumeration
			switch( usb_hstd_enum_seq )  {
			case 1:
				usb_hstd_enum_set_address( 0, 0 );
				break;
			case 5:
				break;
			case 6:
				descriptor_table = &g_usb_hstd_config_descriptor[0].byte;
				g_usb_hstd_device_info[2] = descriptor_table[5];	// Device state
				usb_hstd_enum_set_configuration( descriptor_table[5], 0 );
				break;
			default:
				g_usb_hstd_enumaration_process[usb_hstd_enum_seq]( g_usb_hstd_device_addr, usb_hstd_enum_seq );
			break;
			}
		}
		break;
	case USB_DATA_ERR:
		USB_PRINTF0("### Enumeration is stopped(SETUP or DATA-ERROR)\n");
		usb_hstd_enumeration_err( usb_hstd_enum_seq );
		break;
	case USB_DATA_OVR:
		USB_PRINTF0("### Enumeration is stopped(receive data over)\n");
		usb_hstd_enumeration_err( usb_hstd_enum_seq );
		break;
        case USB_DATA_STALL:
		USB_PRINTF0("### Enumeration is stopped(SETUP or DATA-STALL)\n");
		usb_hstd_enumeration_err( usb_hstd_enum_seq );
		break;
	default:
		USB_PRINTF0("### Enumeration is stopped(result error)\n");
		usb_hstd_enumeration_err( usb_hstd_enum_seq );
		break;
	}
	return enume_mode;
}

/******************************************************************************
 Function Name   : usb_hstd_enumeration_err
 Description     : Output error information when enumeration error occurred.
 Argument        : uint16_t Rnum	: enumeration sequence
 Return          : none
 ******************************************************************************/
static void usb_hstd_enumeration_err(uint16_t Rnum)
{
// Condition compilation by the difference of useful function
#if defined(USB_DEBUG_ON)
	switch( Rnum )  {
	case 0: USB_PRINTF0(" Get_DeviceDescrip(8)\n");		break;
	case 1: USB_PRINTF0(" Set_Address\n");			break;
	case 2: USB_PRINTF0(" Get_DeviceDescrip(18)\n");	break;
	case 3: USB_PRINTF0(" Get_ConfigDescrip(9)\n");		break;
	case 4: USB_PRINTF0(" Get_ConfigDescrip(xx)\n");	break;
	case 5: USB_PRINTF0(" Get_DeviceDescrip(8-2)\n");	break;
	case 6: USB_PRINTF0(" Set_Configuration\n");		break;
	}
#endif	/* defined(USB_DEBUG_ON) */
}

/******************************************************************************
 Function Name   : usb_hstd_chk_device_class
 Description     : Interface class search.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
 Return          : uint16_t		: USB_OK / USB_ERROR
 ******************************************************************************/
static uint16_t usb_hstd_chk_device_class(usb_utr_t *msg)
{
uint8_t *descriptor_table;
uint16_t total_length1;
uint16_t total_length2;
uint16_t result;
uint16_t hub_device;
uint16_t *table[9];
uint16_t tmp4;
uint16_t tmp5;
uint16_t tmp6;
	descriptor_table = &g_usb_hstd_device_descriptor[0].byte;
	/* Device class check */
	tmp4 = descriptor_table[USB_DEV_B_DEVICE_CLASS];
	tmp5 = descriptor_table[USB_DEV_B_DEVICE_SUBCLASS];
	tmp6 = descriptor_table[USB_DEV_B_DEVICE_PROTOCOL];
	hub_device = USB_OK;
	if( 0xFF == tmp4 && 0xFF == tmp5 && 0xFF == tmp6)
		USB_PRINTF0("*** Vendor specific device\n");
	else if( tmp4 || tmp5 || tmp6 )
		USB_PRINTF0("### Device class information error!\n");
	descriptor_table = &g_usb_hstd_config_descriptor[0].byte;
	total_length1 = 0;
	total_length2 = descriptor_table[USB_DEV_W_TOTAL_LENGTH_L] + (descriptor_table[USB_DEV_W_TOTAL_LENGTH_H] << 8);
	if( total_length2 > USB_CONFIGSIZE )
		total_length2 = USB_CONFIGSIZE;
	// Search within configuration total-length
	while( total_length1 < total_length2 )  {
		switch( descriptor_table[total_length1 + 1] )  {
		case USB_DT_CONFIGURATION:		// Configuration Descriptor ?
			table[1] = (uint16_t*)&descriptor_table[total_length1];
			break;
		case USB_DT_INTERFACE:			// Interface Descriptor ?
			if( descriptor_table[total_length1 + 5] == USB_IFCLS_MAS )  {
				result = USB_ERROR;
				table[0] = &g_usb_hstd_device_descriptor[0].word;
				table[2] = (uint16_t*)&descriptor_table[total_length1];
				table[3] = &result;
				table[4] = &hub_device;
				table[6] = &g_usb_hstd_device_speed;
				table[7] = &g_usb_hstd_device_addr;
				usb_hmsc_class_check( table );		// Interface Class
				g_usb_hstd_device_info[3] = descriptor_table[total_length1+5];
				return result;
			}
			break;
		}
		total_length1 += descriptor_table[total_length1];
		if( ! descriptor_table[total_length1] )
			break;
	}
	return USB_ERROR;
}

/******************************************************************************
 Function Name   : usb_hstd_notif_ator_detach
 Description     : Notify MGR (manager) task that attach or detach occurred.
 Arguments       : uint16_t result	: Result
 Return          : none
 ******************************************************************************/
void usb_hstd_notif_ator_detach(uint16_t result)
{
	usb_hstd_mgr_snd_mbx( USB_MSG_MGR_AORDETACH, USB_NULL, result );
}

/******************************************************************************
 Function Name   : usb_hstd_ovcr_notifiation
 Description     : Notify MGR (manager) task that overcurrent was generated.
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_ovcr_notifiation(void)
{
	usb_hstd_mgr_snd_mbx( USB_MSG_MGR_OVERCURRENT, USB_NULL, 0 );
}

/******************************************************************************
 Function Name   : usb_hstd_status_result
 Description     : This is a callback as a result of calling 
                   usb_hstd_change_device_state. This notifies the MGR (manager) 
                   task that the change of USB Device status completed.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure
                 : uint16_t  data	: No use
                 : uint16_t  result	: Result
 Return          : none
 ******************************************************************************/
void usb_hstd_status_result(usb_utr_t *msg, uint16_t data, uint16_t result)
{
	usb_hstd_mgr_snd_mbx( USB_MSG_MGR_STATUSRESULT, USB_NULL, result );
}

/******************************************************************************
 Function Name   : usb_hstd_submit_result
 Description     : Callback after completion of a standard request.
 Arguments       : usb_utr_t *msg	: Pointer to usb_utr_t structure.
                 : uint16_t  data1	: Not used
                 : uint16_t  data2	: Not used
 Return          : none
 ******************************************************************************/
static void usb_hstd_submit_result(usb_utr_t *msg, uint16_t data1, uint16_t data2)
{
	usb_hstd_mgr_snd_mbx( USB_MSG_MGR_SUBMITRESULT, msg->keyword, msg->status );
}

/******************************************************************************
 Function Name   : usb_hstd_enum_get_descriptor
 Description     : Send GetDescriptor to the connected USB device.
 Arguments       : uint16_t devadr	: Device Address
                 : uint16_t cnt_value	: Enumeration sequence
 Return          : none
 ******************************************************************************/
void usb_hstd_enum_get_descriptor(uint16_t devadr, uint16_t cnt_value)
{
uint8_t   *data_table;
uint16_t  *temp;
usb_utr_t *msg;
	switch( cnt_value )  {
	default:
//	case 0:		// continue
//	case 1:		// continue
//	case 5:
		usb_hstd_std_request[0] = USB_GET_DESCRIPTOR | USB_DEV_TO_HOST | USB_STANDARD | USB_DEVICE;
		usb_hstd_std_request[1] = USB_DEV_DESCRIPTOR;
		usb_hstd_std_request[2] = 0x0000;
		usb_hstd_std_request[3] = 0x0040;
		if( usb_hstd_std_request[3] > USB_DEVICESIZE )
			usb_hstd_std_request[3] = USB_DEVICESIZE;
		temp = &g_usb_hstd_device_descriptor[0].word;
		break;
	case 2:
		usb_hstd_std_request[0] = USB_GET_DESCRIPTOR | USB_DEV_TO_HOST | USB_STANDARD | USB_DEVICE;
		usb_hstd_std_request[1] = USB_DEV_DESCRIPTOR;
		usb_hstd_std_request[2] = 0x0000;
		usb_hstd_std_request[3] = 0x0012;
		if( usb_hstd_std_request[3] > USB_DEVICESIZE )
			usb_hstd_std_request[3] = USB_DEVICESIZE;
		temp = &g_usb_hstd_device_descriptor[0].word;
		break;
	case 3:
		usb_hstd_std_request[0] = USB_GET_DESCRIPTOR | USB_DEV_TO_HOST | USB_STANDARD | USB_DEVICE;
		usb_hstd_std_request[1] = USB_CONF_DESCRIPTOR;
		usb_hstd_std_request[2] = 0x0000;
		usb_hstd_std_request[3] = 0x0009;
		temp = &g_usb_hstd_config_descriptor[0].word;
		break;
	case 4:
		data_table = (uint8_t*) g_usb_hstd_config_descriptor;
		usb_hstd_std_request[0] = USB_GET_DESCRIPTOR | USB_DEV_TO_HOST | USB_STANDARD | USB_DEVICE;
		usb_hstd_std_request[1] = USB_CONF_DESCRIPTOR;
		usb_hstd_std_request[2] = 0x0000;
		usb_hstd_std_request[3] = ( data_table[3] << 8 ) + data_table[2];
		if( usb_hstd_std_request[3] > USB_CONFIGSIZE )
			usb_hstd_std_request[3] = USB_CONFIGSIZE;
		temp = &g_usb_hstd_config_descriptor[0].word;
		break;
	}
	usb_hstd_std_request[4] = devadr;
	msg = USB_GET_BLK( );			// Get Message Block
	msg->keyword  = USB_PIPE0;
	msg->tranlen  = usb_hstd_std_request[3];
	msg->setup    = usb_hstd_std_request;
	msg->status   = USB_DATA_NONE;
	msg->complete = usb_hstd_submit_result;
	msg->segment  = USB_TRAN_END;
	msg->tranadr  = temp;
	usb_hstd_transfer_start_req( msg );
}

/******************************************************************************
 Function Name   : usb_hstd_enum_set_address
 Description     : Send SetAddress to the connected USB device.
 Arguments       : uint16_t data1	:
                 : uint16_t data2	:
 Return          : none
 ******************************************************************************/
static void usb_hstd_enum_set_address(uint16_t data1, uint16_t data2)
{
usb_utr_t *msg;
	msg = USB_GET_BLK( );			// Get Message Block
	usb_hstd_std_request[0] = USB_SET_ADDRESS | USB_HOST_TO_DEV | USB_STANDARD | USB_DEVICE;
	usb_hstd_std_request[1] = USB_DEVICE1;
	usb_hstd_std_request[2] = 0x0000;
	usb_hstd_std_request[3] = 0x0000;
	usb_hstd_std_request[4] = USB_DEVICE0;
	msg->keyword  = USB_PIPE0;
	msg->tranadr  = USB_NULL;
	msg->tranlen  = 0;
	msg->setup    = usb_hstd_std_request;
	msg->status   = USB_DATA_NONE;
	msg->complete = usb_hstd_submit_result;
	msg->segment  = USB_TRAN_END;
	usb_hstd_transfer_start_req( msg );
}

/******************************************************************************
 Function Name   : usb_hstd_enum_set_configuration
 Description     : Send SetConfiguration to the connected USB device.
 Arguments       : uint16_t confnum	: Configuration number
                 : uint16_t data2	:
 Return          : none
 ******************************************************************************/
static void usb_hstd_enum_set_configuration(uint16_t confnum, uint16_t data2)
{
usb_utr_t *msg;
	msg = USB_GET_BLK( );			// Get Message Block
	usb_hstd_std_request[0] = USB_SET_CONFIGURATION | USB_HOST_TO_DEV | USB_STANDARD | USB_DEVICE;
	usb_hstd_std_request[1] = confnum;
	usb_hstd_std_request[2] = 0x0000;
	usb_hstd_std_request[3] = 0x0000;
	usb_hstd_std_request[4] = USB_DEVICE1;
	msg->keyword  = USB_PIPE0;
	msg->tranadr  = USB_NULL;
	msg->tranlen  = 0;
	msg->setup    = usb_hstd_std_request;
	msg->status   = USB_DATA_NONE;
	msg->complete = usb_hstd_submit_result;
	msg->segment  = USB_TRAN_END;
	usb_hstd_transfer_start_req( msg );
}

/******************************************************************************
 Function Name   : usb_hstd_enum_dummy_request
 Description     : Dummy function.
 Arguments       : uint16_t data1	:
                 : uint16_t data2	:
 Return          : none
 ******************************************************************************/
void usb_hstd_enum_dummy_request(uint16_t data1, uint16_t data2)
{
}

/******************************************************************************
 Function Name   : usb_hstd_mgr_task
 Description     : The host manager (MGR) task.
 Argument        : usb_utr_t *msg	: Message Address
 Return          : none
 ******************************************************************************/
void usb_hstd_mgr_task(usb_utr_t *msg)
{
        /* Detach is all device */
	switch( msg->msginfo )  {
	case USB_MSG_MGR_STATUSRESULT:		// USB-bus control (change device state)
		switch( g_usb_hstd_mgr_mode )  {
		case USB_DEFAULT:	// End of reset signal
			g_usb_hstd_device_speed = msg->result;
			usb_hstd_set_dev_addr( USB_DEVICE0, g_usb_hstd_device_speed );	// Set device speed
			g_usb_hstd_dcp_register[0] = USB_DEFPACKET + USB_DEVICE0;
			usb_hstd_enum_seq = 0;
			switch( g_usb_hstd_device_speed )  {
			case USB_HSCONNECT:	// Hi Speed Device Connect
				USB_PRINTF0(" Hi-Speed Device\n");
				usb_hstd_enum_get_descriptor( USB_DEVICE0, 0 );
				break;
			case USB_FSCONNECT:	// Full Speed Device Connect
				USB_PRINTF0(" Full-Speed Device\n");
				usb_hstd_enum_get_descriptor( USB_DEVICE0, 0 );
				break;
			case USB_LSCONNECT:	// Low Speed Device Connect
				USB_PRINTF0(" Low-Speed Device\n");
#if defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined(BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671)
				usb_hstd_enum_get_descriptor( USB_DEVICE0, 0 );
#else	/* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX72T)\
    || defined(BSP_MCU_RX72M) || defined (BSP_MCU_RX72N) || defined (BSP_MCU_RX66N) || defined(BSP_MCU_RX671) */
				g_usb_hstd_mgr_mode = USB_DETACHED;
				USB_NoSupportEvent( );				// Set USB Event(No Support)
#endif	/* defined(BSP_MCU_RX64M) || defined(BSP_MCU_RX65N) || defined(BSP_MCU_RX71M) || defined(BSP_MCU_RX671) */
				break;
			default :
				USB_PRINTF0(" Device/Detached\n");
				g_usb_hstd_mgr_mode = USB_DETACHED;
				break;
			}
			break;
		}
		break;
	case USB_MSG_MGR_SUBMITRESULT:		// Agreement device address
		switch( g_usb_hstd_mgr_mode )  {	// Get root port number from device addr
		case USB_DEFAULT:		// Enumeration
			if( USB_NOCONNECT != usb_hstd_support_speed_check( ) )	// Peripheral Device Speed support check
				switch( usb_hstd_enumeration( msg ) )  {	// Detach Mode 
				case USB_NONDEVICE :
					USB_PRINTF0("### Enumeration error\n");
					g_usb_hstd_mgr_mode = USB_DETACHED;
					break;
				case USB_NOTTPL:		// Detach Mode
					USB_PRINTF0("### Not support device\n");
					g_usb_hstd_mgr_mode = USB_DETACHED;
					break;
				case USB_COMPLETEPIPESET:
					g_usb_hstd_mgr_mode = USB_CONFIGURED;
					break;
				}
			break;
		}
		break;
	case USB_MSG_MGR_AORDETACH:
		switch( msg->result )  {
		case USB_DETACH:
			g_usb_hstd_mgr_mode = USB_DETACHED;
			g_usb_hstd_device_speed = USB_NOCONNECT;
			if( USB_DEVICE1 == g_usb_hstd_device_addr )  {
				usb_hmsc_detach( USB_DEVICE1 );
				g_usb_hstd_device_info[0] = USB_NOPORT;		// Root port
				g_usb_hstd_device_info[1] = USB_DETACHED;	// Device state
				g_usb_hstd_device_info[2] = 0;			// Not configured
				g_usb_hstd_device_info[3] = USB_IFCLS_NOT;	// Interface Class : NO class
				g_usb_hstd_device_info[4] = USB_NOCONNECT;	// No connect
				g_usb_hstd_device_state   = USB_DETACHED;	// Device state
				g_usb_hstd_device_addr    = USB_DEVICE0;	// Clear Device Address
			}
			break;
		case USB_ATTACHL:
		case USB_ATTACHF:	// continue
			if( USB_DETACHED == g_usb_hstd_mgr_mode )  {
				g_usb_hstd_device_addr = USB_DEVICE1;		// Set Device Address
				g_usb_hstd_mgr_mode = USB_DEFAULT;
				usb_cpu_delay_xms( 100 );			// Wait 100ms
				usb_hstd_hcd_snd_mbx( USB_MSG_HCD_USBRESET, USB_NULL, 0, usb_hstd_status_result );
			}
		}
		break;
	case USB_MSG_MGR_OVERCURRENT:
		USB_NoSupportEvent( );						// Set USB Event(No Support)
		USB_PRINTF0(" Please detach device \n");
		usb_hstd_vbus_control( USB_VBOFF );
		g_usb_hstd_mgr_mode       = USB_DEFAULT;
		g_usb_hstd_device_info[0] = USB_NOPORT;				// Root port
		g_usb_hstd_device_info[1] = USB_DETACHED;			// Device state
		g_usb_hstd_device_info[2] = 0;					// Not configured
		g_usb_hstd_device_info[3] = USB_IFCLS_NOT;			// Interface Class : NO class
		g_usb_hstd_device_info[4] = USB_NOCONNECT;			// No connect
		g_usb_hstd_device_state   = USB_DETACHED;			// Device state
		break;
	}
	USB_REL_BLK( msg );
}

/******************************************************************************
 Function Name   : usb_hstd_mgr_open
 Description     : Initialize global variable that contains registration status
                   of HDCD.
 Arguments       : none
 Return          : none
 ******************************************************************************/
void usb_hstd_mgr_open(void)
{
	g_usb_hstd_mgr_mode       = USB_DETACHED;	// MGR Mode
	g_usb_hstd_device_state   = USB_DETACHED;	// Device state
	g_usb_hstd_device_info[0] = USB_NOPORT;		// Root port
	g_usb_hstd_device_info[1] = USB_DETACHED;	// Device state
	USB_PRINTF0("*** Install USB-MGR ***\n");
	usb_cstd_set_task_pri( USB_MGR_TSK, USB_PRI_2 );
}
