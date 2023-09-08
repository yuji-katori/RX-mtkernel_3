/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_basic_define.h
 */

#ifndef R_USB_BASIC_DEFINE_H
#define R_USB_BASIC_DEFINE_H

#define USB_CFG_LITTLE          (0)
#define USB_CFG_BIG             (1)

#if defined(__CCRX__)

  #ifdef __BIG
    #define    USB_CFG_ENDIAN           (USB_CFG_BIG)
  #else   /* __BIG */
    #define    USB_CFG_ENDIAN           (USB_CFG_LITTLE)
  #endif  /* __BIG */

#elif defined(__GNUC__)

  #ifdef __RX_BIG_ENDIAN__
    #define    USB_CFG_ENDIAN           (USB_CFG_BIG)
  #else   /* __RX_BIG_ENDIAN__ */
    #define    USB_CFG_ENDIAN           (USB_CFG_LITTLE)
  #endif  /* __RX_BIG_ENDIAN__ */

#elif defined(__ICCRX__)

  #if __BIG_ENDIAN__
    #define    USB_CFG_ENDIAN           (USB_CFG_BIG)
  #else   /* __BIG_ENDIAN__ */
    #define    USB_CFG_ENDIAN           (USB_CFG_LITTLE)
  #endif  /* __BIG_ENDIAN__ */

#endif /* defined(__CCRX__), defined(__GNUC__), defined(__ICCRX__) */

#if defined(BSP_MCU_RX631)
    #define BSP_MCU_RX63N BSP_MCU_RX631
#endif /* #if defined(BSP_MCU_RX631) */

#if defined(BSP_MCU_RX621)
    #define BSP_MCU_RX62N BSP_MCU_RX621
#endif /* #if defined(BSP_MCU_RX631) */

#if defined(BSP_MCU_RX66T)
    #define BSP_MCU_RX72T BSP_MCU_RX66T
#endif /* #if defined(BSP_MCU_RX72T) */

// Scheduler use define
#define USB_BLKMAX		(3)		// Maximum block
#define	USB_TABLECLR		(0)		// Table clear
#define	USB_BUFSIZE		(64)		// Buffer size

// Task priority define
#define	USB_PRI_0		(0)		// Priority 0
#define	USB_PRI_1		(1)		// Priority 1
#define	USB_PRI_2		(2)		// Priority 2
#define	USB_PRI_3		(3)		// Priority 3

// Host Control Driver Task
#define	USB_HCD_TSK		(0)		// Task ID
#define	USB_HCD_MBX		(1)		// Mailbox ID

// Host Manager Task
#define	USB_MGR_TSK		(2)		// Task ID
#define	USB_MGR_MBX		(2)		// Mailbox ID

// H/W function type
#define	USB_BITSET(x)		(1<<(x))

// nonOS Use
#define	USB_SEQ_0		(0x0000)
#define	USB_SEQ_1		(0x0001)
#define	USB_SEQ_2		(0x0002)
#define	USB_SEQ_3		(0x0003)
#define	USB_SEQ_4		(0x0004)
#define	USB_SEQ_5		(0x0005)
#define	USB_SEQ_6		(0x0006)
#define	USB_SEQ_7		(0x0007)

// Interrupt message num
#define	USB_INTMSGMAX		(2)

#define	USB_OK			(0)
#define	USB_ERROR		(0xFF)
#define	USB_QOVR		(0xD5)		// Submit overlap error
#define	USB_PAR			(0xEF)		// parameter error

#define	USB_TRUE		(1)
#define	USB_FALSE		(0)

#define	USB_CFG_HIGH		(1)
#define	USB_CFG_LOW		(0)

// FIFO port register default access size
#define	USB0_CFIFO_MBW		(USB_MBW_16)
#define	USB0_D0FIFO_MBW		(USB_MBW_16)
#define	USB0_D1FIFO_MBW		(USB_MBW_16)

// Pipe Number
#define	USB_MAX_PIPE		(2)

#define	USB_ADDRESS_MASK	(0x000F)

#define	USB_PIPE_DIR_IN		(0)
#define	USB_PIPE_DIR_OUT	(1)
#define	USB_PIPE_DIR_MAX	(2)

#define	USB_CFG_ENABLE		(1)
#define	USB_CFG_DISABLE		(0)

#define	USB_CFG_24MHZ		(0)
#define	USB_CFG_20MHZ		(1)
#define	USB_CFG_OTHER		(2)

// Standard Device Descriptor Define
#define	USB_DEV_B_DESCRIPTOR_TYPE	(1)		// Descriptor type
#define	USB_DEV_B_DEVICE_CLASS		(4)		// Class code
#define	USB_DEV_B_DEVICE_SUBCLASS	(5)		// Subclass code
#define	USB_DEV_B_DEVICE_PROTOCOL	(6)		// Protocol code
#define	USB_DEV_I_PRODUCT		(15)		// Index of string descriptor describing product

// Standard Configuration Descriptor Define
#define	USB_DEV_W_TOTAL_LENGTH_L	(2)		// Total length of data returned for this configuration
#define	USB_DEV_W_TOTAL_LENGTH_H	(3)		// Total length of data returned for this configuration

// Endpoint Descriptor  Define
#define	USB_EP_DIRMASK			(0x80)		// Endpoint direction mask [2]
#define	USB_EP_NUMMASK			(0x0F)		// Endpoint number mask [2]
#define	USB_EP_TRNSMASK			(0x03)		// Transfer type mask [2]

#define	USB_EP_B_ENDPOINTADDRESS	(2)		// Endpoint No. , Dir
#define	USB_EP_B_ATTRIBUTES		(3)		// Transfer Type
#define	USB_EP_B_MAXPACKETSIZE_L	(4)		// Max packet size
#define	USB_EP_B_MAXPACKETSIZE_H	(5)		// Max packet size
#define	USB_EP_B_INTERVAL		(6)		// Interval

// Standard Interface Descriptor Offset Define
#define	USB_IF_B_NUMENDPOINTS		(4)		// bNumEndpoints
#define	USB_IF_B_INTERFACECLASS		(5)		// bInterfaceClass
#define	USB_IF_B_INTERFACESUBCLASS	(6)		// bInterfaceSubClass
#define	USB_IF_B_INTERFACEPROTOCOL	(7)		// bInterfacePtorocol

// CLEAR_FEATURE/GET_FEATURE/SET_FEATURE request information
// Standard Feature Selector
#define	USB_ENDPOINT_HALT		(0x0000)
#define	USB_DEV_REMOTE_WAKEUP		(0x0001)

// GET_DESCRIPTOR/SET_DESCRIPTOR request information
// Standard Descriptor type
#define	USB_DEV_DESCRIPTOR		(0x0100)
#define	USB_CONF_DESCRIPTOR		(0x0200)
#define	USB_STRING_DESCRIPTOR		(0x0300)

// Device connect information
#define	USB_ATTACH			(0x0040)
#define	USB_ATTACHL			(0x0041)
#define	USB_ATTACHF			(0x0042)
#define	USB_DETACH			(0x0043)

// Reset Handshake result
#define	USB_NOCONNECT			(0x0000)	// Speed undecidedness
#define	USB_HSCONNECT			(0x00C0)	// Hi-Speed connect
#define	USB_FSCONNECT			(0x0080)	// Full-Speed connect
#define	USB_LSCONNECT			(0x0040)	// Low-Speed connect

// Pipe configuration table define
#define	USB_TYPFIELD			(0xC000)	// Transfer type
#define	USB_TYPFIELD_ISO		(0xC000)	// Isochronous
#define	USB_TYPFIELD_INT		(0x8000)	// Interrupt
#define	USB_TYPFIELD_BULK		(0x4000)	// Bulk
#define	USB_NOUSE			(0x0000)	// Not configuration
#define	USB_BFREON			(0x0400)
#define	USB_CFG_DBLBON			(0x0200)
#define	USB_CNTMDFIELD			(0x0100)	// Continuous transfer mode select
#define	USB_CFG_CNTMDON			(0x0100)
#define	USB_SHTNAKFIELD			(0x0080)	// Transfer end NAK
#define	USB_DIRFIELD			(0x0010)	// Transfer direction select
#define	USB_DIR_H_OUT			(0x0010)	// HOST OUT
#define	USB_DIR_H_IN			(0x0000)	// HOST IN
#define	USB_EPNUMFIELD			(0x000F)	// Endpoint number select
#define USB_IITVFIELD			(0x0007)	// Isochronous interval

// FIFO port & access define
#define	USB_CUSE			(0)		// CFIFO  trans
#define	USB_D0USE			(1)		// D0FIFO trans

// FIFO read / write result
#define	USB_FIFOERROR			(USB_ERROR)	// FIFO not ready
#define	USB_WRITEEND			(0x0000)	// End of write (but packet may not be outputting)
#define	USB_WRITESHRT			(0x0001)	// End of write (send short packet)
#define	USB_WRITING			(0x0002)	// Write continues
#define	USB_READEND			(0x0000)	// End of read
#define	USB_READSHRT			(0x0001)	// Insufficient (receive short packet)
#define	USB_READING			(0x0002)	// Read continues
#define	USB_READOVER			(0x0003)	// Buffer size over

// Transfer status Type
#define	USB_CTRL_END			(0)
#define	USB_DATA_NONE			(1)
#define	USB_DATA_WAIT			(2)
#define	USB_DATA_OK			(3)
#define	USB_DATA_SHT			(4)
#define	USB_DATA_OVR			(5)
#define	USB_DATA_STALL			(6)
#define	USB_DATA_ERR			(7)
#define	USB_DATA_STOP			(8)
#define	USB_DATA_TMO			(9)
#define	USB_CTRL_READING		(17)
#define	USB_CTRL_WRITING		(18)

// Utr member (segment)
#define	USB_TRAN_CONT			(0x00)
#define	USB_TRAN_END			(0x80)

// Callback argument
#define	USB_NO_ARG			(0)

// USB interrupt type (common)
#define	USB_INT_UNKNOWN			(0x0000)
#define	USB_INT_BRDY			(0x0001)
#define	USB_INT_BEMP			(0x0002)
#define	USB_INT_NRDY			(0x0003)

// USB interrupt type
#define	USB_INT_VBINT			(0x0011)
#define	USB_INT_SOFR			(0x0013)

// USB interrupt type
#define	USB_INT_OVRCR			(0x0041)
#define	USB_INT_BCHG			(0x0042)
#define	USB_INT_DTCH			(0x0043)
#define	USB_INT_ATTCH			(0x0044)
#define	USB_INT_EOFERR			(0x0045)
#define	USB_INT_SACK			(0x0061)
#define	USB_INT_SIGN			(0x0062)

#define	USB_VBON			(1)
#define	USB_VBOFF			(0)

// Root port
#define	USB_NOPORT			(0xFFFF)	// Not connect

// Condition compilation by the difference of IP
#define	USB_MAXDEVADR			(2)

#define	USB_DEVICE0			(0x0000)	// Device address 0
#define	USB_DEVICE1			(0x0001)	// Device address 1
#define	USB_NODEVICE			(0xF000)	// No device
#define	USB_DEVADDRBIT			(12)

// DCP Max packet size
#define	USB_MAXPFIELD			(0x007F)	// Max packet size of DCP

// ControlPipe Max Packet size
#define	USB_DEFPACKET			(0x0040)	// Default DCP Max packet size

// Device state define
#define	USB_NONDEVICE			(0)
#define	USB_NOTTPL			(1)
#define	USB_DEVICEENUMERATION		(3)
#define	USB_COMPLETEPIPESET		(10)

// Control Transfer Stage
#define	USB_IDLEST			(0)		// Idle
#define	USB_SETUPNDC			(1)		// Setup Stage No Data Control
#define	USB_SETUPWR			(2)		// Setup Stage Control Write
#define	USB_SETUPRD			(3)		// Setup Stage Control Read
#define	USB_DATAWR			(4)		// Data Stage Control Write
#define	USB_DATARD			(5)		// Data Stage Control Read
#define	USB_STATUSRD			(6)		// Status stage
#define	USB_STATUSWR			(7)		// Status stage
#define	USB_SETUPWRCNT			(17)		// Setup Stage Control Write
#define	USB_SETUPRDCNT			(18)		// Setup Stage Control Read
#define	USB_DATAWRCNT			(19)		// Data Stage Control Write
#define	USB_DATARDCNT			(20)		// Data Stage Control Read

//  USB Manager mode
#define	USB_DETACHED			(10)		// Disconnect(VBUSon)
#define	USB_DEFAULT			(40)		// Set device address
#define	USB_CONFIGURED			(70)		// Detach detected
#define	USB_SUSPENDED			(80)		// Device suspended
#define	USB_SUSPENDED_PROCESS		(102)		// Wait device suspend
#define	USB_RESUME_PROCESS		(103)		// Wait device resume

// HCD common task message command
#define	USB_MSG_HCD_ATTACH		(0x0101)
#define	USB_MSG_HCD_DETACH		(0x0102)
#define	USB_MSG_HCD_USBRESET		(0x0103)
#define	USB_MSG_HCD_SUSPEND		(0x0104)
#define	USB_MSG_HCD_RESUME		(0x0105)
#define	USB_MSG_HCD_REMOTE		(0x0106)
#define	USB_MSG_HCD_VBON		(0x0107)
#define	USB_MSG_HCD_VBOFF		(0x0108)
#define	USB_MSG_HCD_CLR_STALL		(0x0109)
#define	USB_MSG_HCD_DETACH_MGR		(0x010A)
#define	USB_MSG_HCD_ATTACH_MGR		(0x010B)
#define	USB_MSG_HCD_CLR_STALL_RESULT	(0x010C)
#define	USB_MSG_HCD_CLR_STALLBIT	(0x010D)
#define	USB_MSG_HCD_SQTGLBIT		(0x010F)

// HCD task message command
#define	USB_MSG_HCD_SUBMITUTR		(0x0112)
#define	USB_MSG_HCD_CLRSEQBIT		(0x0115)
#define	USB_MSG_HCD_SETSEQBIT		(0x0116)
#define	USB_MSG_HCD_INT			(0x0117)
#define	USB_MSG_HCD_DMAINT		(0x0119)

// USB Manager task message command
#define	USB_MSG_MGR_AORDETACH		(0x0121)
#define	USB_MSG_MGR_OVERCURRENT		(0x0122)
#define	USB_MSG_MGR_STATUSRESULT	(0x0123)
#define	USB_MSG_MGR_SUBMITRESULT	(0x0124)

// CLS task message command
#define	USB_MSG_CLS_INIT		(0x0202)

// USB Peripheral task message command
#define	USB_DO_RESET_AND_ENUMERATION	(0x0202)	// USB_MSG_HCD_ATTACH
#define	USB_PORT_ENABLE			(0x0203)	// USB_MSG_HCD_VBON
#define	USB_PORT_DISABLE		(0x0204)	// USB_MSG_HCD_VBOFF
#define	USB_DO_GLOBAL_SUSPEND		(0x0205)	// USB_MSG_HCD_SUSPEND
#define	USB_DO_SELECTIVE_SUSPEND	(0x0206)	// USB_MSG_HCD_SUSPEND
#define	USB_DO_GLOBAL_RESUME		(0x0207)	// USB_MSG_HCD_RESUME
#define	USB_DO_SELECTIVE_RESUME		(0x0208)	// USB_MSG_HCD_RESUME
#define	USB_DO_CLR_STALL		(0x0209)	// USB_MSG_HCD_CLR_STALL
#define	USB_DO_SET_SQTGL		(0x020A)	// USB_MSG_HCD_SQTGLBIT
#define	USB_DO_CLR_SQTGL		(0x020B)	// USB_MSG_HCD_CLRSEQBIT

#define	USB_SND_MSG(ID, MSG)		(usb_cstd_snd_msg((ID), (MSG)))
#define	USB_GET_BLK(BLK)		(usb_cstd_get_blk( ))
#define	USB_REL_BLK(BLK)		(usb_cstd_rel_blk(BLK))

// Descriptor size
#define	USB_DEVICESIZE			(20)		// Device Descriptor size
#define	USB_CONFIGSIZE			(256)		// Configuration Descriptor size

// Number of software retries when a no-response condition occurs during a transfer
#define	USB_PIPEERROR			(1)

/** [Output debugging message in a console of IDE.]
 *   not defined(USB_DEBUG_ON) : No output the debugging message
 *   defined(USB_DEBUG_ON)     : Output the debugging message
 */
#if defined(USB_DEBUG_ON)
    #define USB_PRINTF0(FORM)                           (tm_printf(FORM))
    #define USB_PRINTF1(FORM,x1)                        (tm_printf((FORM), (x1)))
    #define USB_PRINTF2(FORM,x1,x2)                     (tm_printf((FORM), (x1), (x2)))
    #define USB_PRINTF3(FORM,x1,x2,x3)                  (tm_printf((FORM), (x1), (x2), (x3)))
    #define USB_PRINTF4(FORM,x1,x2,x3,x4)               (tm_printf((FORM), (x1), (x2), (x3), (x4)))
    #define USB_PRINTF5(FORM,x1,x2,x3,x4,x5)            (tm_printf((FORM), (x1), (x2), (x3), (x4), (x5)))
    #define USB_PRINTF6(FORM,x1,x2,x3,x4,x5,x6)         (tm_printf((FORM), (x1), (x2), (x3), (x4), (x5), (x6)))
    #define USB_PRINTF7(FORM,x1,x2,x3,x4,x5,x6,x7)      (tm_printf((FORM), (x1), (x2), (x3), (x4), (x5), (x6), (x7)))
    #define USB_PRINTF8(FORM,x1,x2,x3,x4,x5,x6,x7,x8)   (tm_printf((FORM), (x1), (x2), (x3), (x4), (x5), (x6), (x7), (x8)))
#else	/* defined(USB_DEBUG_ON) */
    #define USB_PRINTF0(FORM)
    #define USB_PRINTF1(FORM,x1)
    #define USB_PRINTF2(FORM,x1,x2)
    #define USB_PRINTF3(FORM,x1,x2,x3)
    #define USB_PRINTF4(FORM,x1,x2,x3,x4)
    #define USB_PRINTF5(FORM,x1,x2,x3,x4,x5)
    #define USB_PRINTF6(FORM,x1,x2,x3,x4,x5,x6)
    #define USB_PRINTF7(FORM,x1,x2,x3,x4,x5,x6,x7)
    #define USB_PRINTF8(FORM,x1,x2,x3,x4,x5,x6,x7,x8)
#endif	/* defined(USB_DEBUG_ON) */

#endif	/* R_USB_BASIC_DEFINE_H */