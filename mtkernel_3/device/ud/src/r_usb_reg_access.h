/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	r_usb_reg_access.h
 */

#ifndef HW_USB_REG_ACCESS_H
#define HW_USB_REG_ACCESS_H

#define	USB_BUFSIZE_BIT		(10)

//  INITIARIZE
void     hw_usb_hmodule_init(void);

//  DVSTCTR0
void     hw_usb_rmw_dvstctr(uint16_t data, uint16_t width);
void     hw_usb_set_vbout(void);
void     hw_usb_clear_vbout(void);

//  CFIFO, D0FIFO, D1FIFO
void     hw_usb_write_fifo8(uint16_t pipemode, uint8_t  data);

//  CFIFOSEL, D0FIFOSEL, D1FIFOSEL
uint16_t hw_usb_read_fifosel(uint16_t pipemode);
void     hw_usb_rmw_fifosel(uint16_t data, uint16_t bitptn);
void     hw_usb_clear_dreqe(void);
void     hw_usb_set_mbw(uint16_t pipemode, uint16_t data);
void     hw_usb_set_curpipe(uint16_t pipemode, uint16_t pipe);

// CFIFOCTR, D0FIFOCTR, D1FIFOCTR
uint16_t hw_usb_read_fifoctr(uint16_t pipemode);
void     hw_usb_set_bval(uint16_t pipemode);
void     hw_usb_set_bclr(uint16_t pipemode);

// DCPCTR, PIPEnCTR
uint16_t hw_usb_read_pipectr(uint16_t pipe);
void     hw_usb_set_sqclr(uint16_t pipe);
void     hw_usb_set_sqset(uint16_t pipe);
void     hw_usb_clear_pid(uint16_t pipe, uint16_t data);

#endif	/* HW_USB_REG_ACCESS_H */
