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
#define		USE_RXD1_P15			/* Use SCI1 RXD1 is P15 */
//#define	USE_RXD1_P30			/* Use SCI1 RXD1 is P30 */
#define		USE_TXD1_P16			/* Use SCI1 TXD1 is P16 */
//#define	USE_TXD1_P26			/* Use SCI1 TXD1 is P26 */

//#define	USE_SCI2			/* Use SCI2 */
#define		USE_RXD2_P12			/* Use SCI2 RXD2 is P12 */
//#define	USE_RXD2_P52			/* Use SCI2 RXD2 is P52 */
#define		USE_TXD2_P13			/* Use SCI2 TXD2 is P13 */
//#define	USE_TXD2_P50			/* Use SCI2 TXD2 is P50 */

//#define	USE_SCI3			/* Use SCI3 */
#define		USE_RXD3_P16			/* Use SCI3 RXD3 is P16 */
//#define	USE_RXD3_P25			/* Use SCI3 RXD3 is P25 */
#define		USE_TXD3_P17			/* Use SCI3 TXD3 is P17 */
//#define	USE_TXD3_P23			/* Use SCI3 TXD3 is P23 */

//#define	USE_SCI4			/* Use SCI4 */
#define		USE_RXD4_PB0			/* Use SCI4 RXD4 is PB0 */
#define		USE_TXD4_PB1			/* Use SCI4 TXD4 is PB1 */

//#define	USE_SCI5			/* Use SCI5 */
#define		USE_RXD5_PA2			/* Use SCI5 RXD5 is PA2 */
//#define	USE_RXD5_PA3			/* Use SCI5 RXD5 is PA3 */
//#define	USE_RXD5_PC2			/* Use SCI5 RXD5 is PC2 */
#define		USE_TXD5_PA4			/* Use SCI5 TXD5 is PA4 */
//#define	USE_TXD5_PC3			/* Use SCI5 TXD5 is PC3 */

//#define	USE_SCI6			/* Use SCI6 */
#define		USE_RXD6_P33			/* Use SCI6 RXD6 is P33 */
//#define	USE_RXD6_PB0			/* Use SCI6 RXD6 is PB0 */
#define		USE_TXD6_P32			/* Use SCI6 TXD6 is P32 */
//#define	USE_TXD6_PB1			/* Use SCI6 TXD6 is PB1 */

//#define	USE_SCI9			/* Use SCI9 */
#define		USE_RXD9_PB6			/* Use SCI9 RXD9 is PB6 */
#define		USE_TXD9_PB7			/* Use SCI9 TXD9 is PB7 */

//#define	USE_SC11			/* Use SCI11 */
#define		USE_RXD11_PB6			/* Use SCI11 RXD11 is PB6 */
#define		USE_TXD11_PB7			/* Use SCI11 TXD11 is PB7 */

//#define	USE_SCI12			/* Use SCI12 */
#define		USE_RXD12_PE2			/* Use SCI12 RXD12 is PE2 */
#define		USE_TXD12_PE1			/* Use SCI12 TXD12 is PE1 */

#endif	/* SCI_CONFIG_H */