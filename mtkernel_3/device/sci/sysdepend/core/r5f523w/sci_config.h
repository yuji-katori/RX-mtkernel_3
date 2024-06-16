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

#define		USE_SCI1			/* Use SCI1 */
#define		USE_RXD1_P15			/* Use SCI1 RXD1 is P15 */
//#define	USE_RXD1_P30			/* Use SCI1 RXD1 is P30 */
#define		USE_TXD1_P16			/* Use SCI1 TXD1 is P16 */
//#define	USE_TXD1_P26			/* Use SCI1 TXD1 is P26 */

//#define	USE_SCI5			/* Use SCI5 */
#define		USE_RXD5_PC2			/* Use SCI5 RXD5 is PC2 */
#define		USE_TXD5_PC3			/* Use SCI5 TXD5 is PC3 */

//#define	USE_SCI8			/* Use SCI8 */
#define		USE_RXD8_PC6			/* Use SCI8 RXD8 is PC6 */
#define		USE_TXD8_PC7			/* Use SCI8 TXD8 is PC7 */

//#define	USE_SCI12			/* Use SCI12 */
#define		USE_RXD12_PE2			/* Use SCI12 RXD12 is PE2 */
#define		USE_TXD12_PE1			/* Use SCI12 TXD12 is PE1 */

#endif	/* SCI_CONFIG_H */