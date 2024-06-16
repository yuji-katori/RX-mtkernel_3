/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	sci_config.h
 */
#ifndef SCI_CONFIG_H
#define SCI_CONFIG_H

/* SCI control task priority. */
#define	SCI_CFG_TASK_PRIORITY				(4)

/* SCI interrupt priority level. */
#define	SCI_CFG_INT_PRIORITY				(9)

/* SCI receive buffer size. */
#define	SCI_CFG_RBUF_SIZE				(32)

#define		USE_SCI0			/* Use SCI0 */
#define		USE_RXD0_P21			/* Use SCI0 RXD0 is P21 */
//#define	USE_RXD0_P33			/* Use SCI0 RXD0 is P33 */
#define		USE_TXD0_P20			/* Use SCI0 TXD0 is P20 */
//#define	USE_TXD0_P32			/* Use SCI0 TXD0 is P32 */

//#define	USE_SCI1			/* Use SCI1 */
#define		USE_RXD1_P30			/* Use SCI1 RXD1 is P30 */
#define		USE_TXD1_P26			/* Use SCI1 TXD1 is P26 */

//#define	USE_SCI3			/* Use SCI3 */
#define		USE_RXD3_P25			/* Use SCI3 RXD3 is P25 */
#define		USE_TXD3_P23			/* Use SCI3 TXD3 is P23 */

//#define	USE_SCI6			/* Use SCI6 */
#define		USE_RXD6_P01			/* Use SCI6 RXD6 is P01 */
//#define	USE_RXD6_P33			/* Use SCI6 RXD6 is P33 */
#define		USE_TXD6_P00			/* Use SCI6 TXD6 is P00 */
//#define	USE_TXD6_P32			/* Use SCI6 TXD6 is P32 */

//#define	USE_SC11			/* Use SCI11 */
#define		USE_RXD11_P76			/* Use SCI11 RXD11 is P76 */
#define		USE_TXD11_P77			/* Use SCI11 TXD11 is P77 */

#endif	/* SCI_CONFIG_H */