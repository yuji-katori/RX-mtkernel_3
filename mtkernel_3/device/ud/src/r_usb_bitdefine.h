/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_bitdefine.h
 */

#ifndef R_USB_BITDEFINE_H
#define R_USB_BITDEFINE_H

// System Configuration Control Register
#define	USB_SCKE                (0x0400)	// b10: USB clock enable
#define	USB_DCFM                (0x0040)	// b6: Function select
#define	USB_DRPD                (0x0020)	// b5: D+/D- pull down control
#define	USB_DPRPU               (0x0010)	// b4: D+ pull up control
#define	USB_USBE                (0x0001)	// b0: USB module enable

// System Configuration Status Register
#define	USB_LNST                (0x0003)	// b1-0: D+, D- line status
#define	USB_FS_JSTS             (0x0001)	// Full-Speed J State
#define	USB_LS_JSTS             (0x0002)	// Low-Speed J State
#define	USB_SE0                 (0x0000)	// SE0

// Device State Control Register
#define	USB_VBUSEN              (0x0200)	// b9: VBUS output terminal control
#define	USB_RWUPE               (0x0080)	// b7: Remote wakeup sense
#define	USB_USBRST              (0x0040)	// b6: USB reset enable
#define	USB_RESUME              (0x0020)	// b5: Resume enable
#define	USB_UACT                (0x0010)	// b4: USB bus enable
#define	USB_RHST                (0x0007)	// b2-0: Reset handshake status
#define	USB_HSPROC              (0x0004)	// HS handshake processing
#define	USB_HSMODE              (0x0003)	// Hi-Speed mode
#define	USB_FSMODE              (0x0002)	// Full-Speed mode
#define	USB_LSMODE              (0x0001)	// Low-Speed mode
#define	USB_UNDECID             (0x0000)	// Undecided

// CFIFO/DxFIFO Port Select Register
#define	USB_RCNT                (0x8000)	// b15: Read count mode
#define	USB_DCLRM               (0x2000)	// b13: Automatic buffer clear mode
#define	USB_DREQE               (0x1000)	// b12: DREQ output enable
#define	USB_MBW_16              (0x0400)	// FIFO access : 16bit
#define	USB_MBW_8               (0x0000)	// FIFO access : 8bit
#define	USB_BIGEND              (0x0100)	// b8: Big endian mode
#define	USB_FIFO_BIG            (0x0100)	// Big endian
#define	USB_FIFO_LITTLE         (0x0000)	// Little endian
#define	USB_ISEL                (0x0020)	// b5: DCP FIFO port direction select
#define	USB_CURPIPE             (0x000F)	// b2-0: PIPE select

// CFIFO/DxFIFO Port Control Register
#define	USB_BVAL                (0x8000)	// b15: Buffer valid flag
#define	USB_BCLR                (0x4000)	// b14: Buffer clear
#define	USB_FRDY                (0x2000)	// b13: FIFO ready
#define	USB_DTLN                (0x0FFF)	// b11-0: FIFO data length

// Interrupt Enable Register 0
#define	USB_VBSE                (0x8000)	// b15: VBUS interrupt
#define	USB_SOFE                (0x2000)	// b13: Frame update interrupt
#define	USB_BEMPE               (0x0400)	// b10: Buffer empty interrupt
#define	USB_NRDYE               (0x0200)	// b9: Buffer notready interrupt
#define	USB_BRDYE               (0x0100)	// b8: Buffer ready interrupt

// Interrupt Enable Register 1
#define	USB_OVRCRE              (0x8000)	// b15: Over-current interrupt
#define	USB_BCHGE               (0x4000)	// b14: USB bus change interrupt
#define	USB_DTCHE               (0x1000)	// b12: Detach sense interrupt
#define	USB_ATTCHE              (0x0800)	// b11: Attach sense interrupt
#define	USB_SIGNE               (0x0020)	// b5: SETUP IGNORE interrupt
#define	USB_SACKE               (0x0010)	// b4: SETUP ACK interrupt

// BRDY Interrupt Enable/Status Register
#define	USB_BRDY0               (0x0001)	// b1: PIPE0

// NRDY Interrupt Enable/Status Register
#define	USB_NRDY0               (0x0001)	// b1: PIPE0

// BEMP Interrupt Enable/Status Register
#define	USB_BEMP0               (0x0001)	// b0: PIPE0

// SOF Pin Configuration Register
#define	USB_TRNENSEL            (0x0100)	// b8: Select transaction enable period

// Interrupt Status Register 0
#define	USB_VBINT               (0x8000)	// b15: VBUS interrupt
#define	USB_SOFR                (0x2000)	// b13: SOF update interrupt
#define	USB_BEMP                (0x0400)	// b10: Buffer empty interrupt
#define	USB_NRDY                (0x0200)	// b9: Buffer notready interrupt
#define	USB_BRDY                (0x0100)	// b8: Buffer ready interrupt

// Interrupt Status Register 1
#define	USB_OVRCR               (0x8000)	// b15: Over-current interrupt
#define	USB_BCHG                (0x4000)	// b14: USB bus change interrupt
#define	USB_DTCH                (0x1000)	// b12: Detach sense interrupt
#define	USB_ATTCH               (0x0800)	// b11: Attach sense interrupt
#define	USB_EOFERR              (0x0040)	// b6: EOF-error interrupt
#define	USB_SIGN                (0x0020)	// b5: Setup ignore interrupt
#define	USB_SACK                (0x0010)	// b4: Setup ack interrupt

// Frame Number Register
#define	USB_OVRN                (0x8000)	// b15: Overrun error

// USB Request Type Register
#define	USB_BMREQUESTTYPEDIR    (0x0080)	// b7 : Data transfer direction

// Refer to r_usb_cdefusbip.h
#define	USB_DEVSEL              (0xF000)	// b15-14: Device address select
#define	USB_MAXP                (0x007F)	// b6-0: Maxpacket size of default control pipe
#define	USB_MXPS                (0x07FF)	// b10-0: Maxpacket size

#define	USB_SUREQ               (0x4000)	// b14: Send USB request 
#define	USB_INBUFM              (0x4000)	// b14: IN buffer monitor (Only for PIPE1 to 5)
#define	USB_SUREQCLR            (0x0800)	// b11: stop setup request
#define	USB_ACLRM               (0x0200)	// b9: buffer auto clear mode
#define	USB_SQCLR               (0x0100)	// b8: Sequence bit clear
#define	USB_SQSET               (0x0080)	// b7: Sequence bit set
#define	USB_SQMON               (0x0040)	// b6: Sequence bit monitor
#define	USB_PBUSY               (0x0020)	// b5: pipe busy
#define	USB_PID                 (0x0003)	// b1-0: Response PID
#define	USB_PID_STALL           (0x0002)	// STALL
#define	USB_PID_BUF             (0x0001)	// BUF

// Refer to r_usb_cdefusbip.h
#define	USB_PIPE0BUF            (256)
#define	USB_TRENB               (0x0200)	// b9: Transaction count enable
#define	USB_TRCLR               (0x0100)	// b8: Transaction count clear
#define	USB_TRNCNT              (0xFFFF)	// b15-0: Transaction counter

// USB IO Register Reserved bit mask
#define	INTSTS1_MASK            (0xD870)	// INTSTS1 Reserved bit mask
#define	BRDYSTS_MASK            (0x03FF)	// BRDYSTS Reserved bit mask
#define	NRDYSTS_MASK            (0x03FF)	// NRDYSTS Reserved bit mask
#define	BEMPSTS_MASK            (0x03FF)	// BEMPSTS Reserved bit mask

#endif /* R_USB_BITDEFINE_H */
