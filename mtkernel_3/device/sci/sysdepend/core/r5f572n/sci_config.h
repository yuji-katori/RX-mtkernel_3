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
//#define	USE_RXD1_PF2			/* Use SCI1 RXD1 is PF2 */
#define		USE_TXD1_P16			/* Use SCI1 TXD1 is P16 */
//#define	USE_TXD1_P26			/* Use SCI1 TXD1 is P26 */
//#define	USE_TXD1_PF0			/* Use SCI1 TXD1 is PF0 */

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
#define		USE_RXD6_P01			/* Use SCI6 RXD6 is P01 */
//#define	USE_RXD6_P33			/* Use SCI6 RXD6 is P33 */
//#define	USE_RXD6_PB0			/* Use SCI6 RXD6 is PB0 */
#define		USE_TXD6_P00			/* Use SCI6 TXD6 is P00 */
//#define	USE_TXD6_P32			/* Use SCI6 TXD6 is P32 */
//#define	USE_TXD6_PB1			/* Use SCI6 TXD6 is PB1 */

//#define	USE_SCI7			/* Use SCI7 */
#define		USE_RXD7_P57			/* Use SCI7 RXD7 is P57 */
//#define	USE_RXD7_P92			/* Use SCI7 RXD7 is P92 */
//#define	USE_RXD7_PH1			/* Use SCI7 RXD7 is PH1 */
#define		USE_TXD7_P55			/* Use SCI7 TXD7 is P55 */
//#define	USE_TXD7_P90			/* Use SCI7 TXD7 is P90 */
//#define	USE_TXD7_PH2			/* Use SCI7 TXD7 is PH2 */

//#define	USE_SCI8			/* Use SCI8 */
#define		USE_RXD8_PC6			/* Use SCI8 RXD8 is PC6 */
//#define	USE_RXD8_PJ1			/* Use SCI8 RXD8 is PJ1 */
//#define	USE_RXD8_PK1			/* Use SCI8 RXD8 is PK1 */
#define		USE_TXD8_PC7			/* Use SCI8 TXD8 is PC7 */
//#define	USE_TXD8_PJ2			/* Use SCI8 TXD8 is PJ2 */
//#define	USE_TXD8_PK2			/* Use SCI8 TXD8 is PK2 */

//#define	USE_SCI9			/* Use SCI9 */
#define		USE_RXD9_PB6			/* Use SCI9 RXD9 is PB6 */
//#define	USE_RXD9_PL1			/* Use SCI9 RXD9 is PL1 */
#define		USE_TXD9_PB7			/* Use SCI9 TXD9 is PB7 */
//#define	USE_TXD9_PL2			/* Use SCI9 TXD9 is PL2 */

//#define	USE_SCI10			/* Use SCI10 */
#define		USE_RXD10_P81			/* Use SCI10 RXD10 is P81 */
//#define	USE_RXD10_P86			/* Use SCI10 RXD10 is P86 */
//#define	USE_RXD10_PC6			/* Use SCI10 RXD10 is PC6 */
//#define	USE_RXD10_PM1			/* Use SCI10 RXD10 is PM1 */
#define		USE_TXD10_P82			/* Use SCI10 TXD10 is P82 */
//#define	USE_TXD10_P87			/* Use SCI10 TXD10 is P87 */
//#define	USE_TXD10_PC7			/* Use SCI10 TXD10 is PC7 */
//#define	USE_TXD10_PM2			/* Use SCI10 TXD10 is PM2 */

//#define	USE_SC11			/* Use SCI11 */
#define		USE_RXD11_P76			/* Use SCI11 RXD11 is P76 */
//#define	USE_RXD11_PB6			/* Use SCI11 RXD11 is PB6 */
//#define	USE_RXD11_PQ1			/* Use SCI11 RXD11 is PQ1 */
#define		USE_TXD11_P77			/* Use SCI11 TXD11 is P77 */
//#define	USE_TXD11_PB7			/* Use SCI11 TXD11 is PB7 */
//#define	USE_TXD11_PQ2			/* Use SCI11 TXD11 is PQ2 */

//#define	USE_SCI12			/* Use SCI12 */
#define		USE_RXD12_PE2			/* Use SCI12 RXD12 is PE2 */
#define		USE_TXD12_PE1			/* Use SCI12 TXD12 is PE1 */

#endif	/* SCI_CONFIG_H */