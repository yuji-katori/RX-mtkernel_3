/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_hmsc.h
 */

#ifndef R_USB_HMSC_LOCAL_H
#define R_USB_HMSC_LOCAL_H

// Host Sample Task
#define	USB_HMSC_TSK		(3)		// Task ID
#define	USB_HMSC_MBX		(3)		// Mailbox ID

// Host Sample Task
#define	USB_HSTRG_TSK		(4)		// Task ID
#define	USB_HSTRG_MBX		(4)		// Mailbox ID

/* CBW definitions */
#define	USB_MSC_CBWLENGTH			(31)
#define	USB_MSC_CBWCB_LENGTH			(12)

/* CPU bit endian select (BIT_LITTLE:little, BIT_BIG:big) */
#if USB_CFG_ENDIAN == USB_CFG_BIG
#define	USB_MSC_CBW_SIGNATURE			(0x55534243)
#else   /* USB_CFG_ENDIAN == USB_CFG_BIG */
#define	USB_MSC_CBW_SIGNATURE			(0x43425355)
#endif  /* USB_CFG_ENDIAN == USB_CFG_BIG */

/* CSW definitions */
#define	USB_MSC_CSW_LENGTH			(13)
/* CPU bit endian select (BIT_LITTLE:little, BIT_BIG:big) */
#if USB_CFG_ENDIAN == USB_CFG_BIG
#define	USB_MSC_CSW_SIGNATURE			(0x55534253)
#else   /* USB_CFG_ENDIAN == USB_CFG_BIG */
#define	USB_MSC_CSW_SIGNATURE			(0x53425355)
#endif  /* USB_CFG_ENDIAN == USB_CFG_BIG */

/* subClass code */
#define USB_ATAPI_MMC5				(0x02)
#define USB_ATAPI				(0x05)
#define USB_SCSI				(0x06)

// Protocol code
#define	USB_BOTP				(0x0050)

// Message code
#define	USB_MSG_HMSC_NO_DATA			(0x0501)
#define	USB_MSG_HMSC_DATA_IN			(0x0502)
#define	USB_MSG_HMSC_DATA_OUT			(0x0503)
#define	USB_MSG_HMSC_CSW_PHASE_ERR		(0x0505)
#define	USB_MSG_HMSC_CBW_ERR			(0x0506)

#define	USB_MSG_HMSC_STRG_DRIVE_SEARCH		(0x0601)
#define	USB_MSG_HMSC_STRG_DRIVE_SEARCH_END	(0x0602)
#define	USB_MSG_HMSC_STRG_USER_COMMAND		(0x0605)
#define	USB_MSG_HMSC_STRG_RW_END		(0x0606)

// Class Request Buffer Size
#define	USB_HMSC_CLSDATASIZE			(256)
#define	USB_HMSC_STRG_SECTSIZE			(512)		// 512 bytes per sector

// CBW Structure define
typedef struct usb_msc_cbw {
	uint32_t	dcbw_signature;
	uint32_t	dcbw_tag;
	uint8_t		dcbw_dtl_lo;
	uint8_t		dcbw_dtl_ml;
	uint8_t		dcbw_dtl_mh;
	uint8_t		dcbw_dtl_hi;
	uint8_t		bm_cbw_flags;
	uint8_t		bcbw_lun;
	uint8_t		bcbwcb_length;
	uint8_t		cbwcb[12];
	uint8_t		work;
} usb_msc_cbw_t;

// CSW Structure define define
typedef struct usb_msc_csw {
	uint32_t	dcsw_signature;
	uint32_t	dcsw_tag;
	uint8_t		dcsw_status;
	uint8_t		work;
} usb_msc_csw_t;

// ERROR CODE
typedef enum {
	USB_HMSC_OK			= 0,
	USB_HMSC_STALL			= 1,
	USB_HMSC_CBW_ERR		= 2,		// CBW error
	USB_HMSC_DAT_RD_ERR		= 3,		// Data IN error
	USB_HMSC_DAT_WR_ERR		= 4,		// Data OUT error
	USB_HMSC_CSW_ERR		= 5,		// CSW error
	USB_HMSC_CSW_PHASE_ERR		= 6,		// Phase error
	USB_HMSC_SUBMIT_ERR		= 7,		// Submit error
}g_usb_hmsc_error_t;

typedef enum {
	USB_ATAPI_TEST_UNIT_READY	= 0x00,
	USB_ATAPI_REQUEST_SENSE		= 0x03,
	USB_ATAPI_FORMAT_UNIT		= 0x04,
	USB_ATAPI_INQUIRY		= 0x12,
	USB_ATAPI_MODE_SELECT6		= 0x15,
	USB_ATAPI_MODE_SENSE6		= 0x1A,
	USB_ATAPI_START_STOP_UNIT	= 0x1B,
	USB_ATAPI_PREVENT_ALLOW		= 0x1E,
	USB_ATAPI_READ_FORMAT_CAPACITY	= 0x23,
	USB_ATAPI_READ_CAPACITY		= 0x25,
	USB_ATAPI_READ10		= 0x28,
	USB_ATAPI_WRITE10		= 0x2A,
	USB_ATAPI_SEEK			= 0x2B,
	USB_ATAPI_WRITE_AND_VERIFY	= 0x2E,
	USB_ATAPI_VERIFY10		= 0x2F,
	USB_ATAPI_MODE_SELECT10		= 0x55,
	USB_ATAPI_MODE_SENSE10		= 0x5A,
} usb_atapi_t;

// CSW STATUS
typedef enum  {
	USB_MSC_CSW_OK, USB_MSC_CSW_NG, USB_MSC_CSW_PHASE_ERR
} usb_gcmsc_cswsts_t;

// HMSC driver
extern uint8_t  g_usb_hmsc_sub_class;
extern uint8_t *g_usb_hmsc_config_table;
extern uint8_t *g_usb_hmsc_interface_table;
extern uint16_t g_usb_hmsc_speed;

// Storage Driver
extern uint16_t g_usb_hmsc_strg_process;

// HMSC driver
uint16_t usb_hmsc_pipe_info(uint8_t *table, uint16_t speed, uint16_t length);
void     usb_hmsc_message_retry(uint16_t id, usb_utr_t *msg);

// Storage Driver
uint16_t usb_hmsc_strg_read_sector(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte);
uint16_t usb_hmsc_strg_write_sector(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte);
void     usb_hmsc_request_sense(uint8_t *buf);
uint8_t  usb_hmsc_strg_drive_open(uint16_t devadr);
void     usb_hmsc_strg_drive_close(void);
usb_er_t usb_hmsc_get_max_unit(uint16_t devadr, uint8_t *buf, usb_cb_t complete);
void     usb_hmsc_inquiry(void);
void     usb_hmsc_read_format_capacity(void);
void     usb_hmsc_read_capacity(void);
void     usb_hmsc_test_unit(void);
void     usb_hmsc_read10(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte);
void     usb_hmsc_write10(uint8_t *buf, uint32_t secno, uint16_t seccnt, uint32_t trans_byte);

#endif  /* R_USB_HMSC_LOCAL_H */
