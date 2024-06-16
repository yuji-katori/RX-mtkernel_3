/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	siic_config.h
 */
#ifndef SIIC_CONFIG_H
#define SIIC_CONFIG_H

/* SIIC control task priority. */
#define	SIIC_CFG_TASK_PRIORITY				(4)

/* SIIC interrupt priority level. */
#define	SIIC_CFG_INT_PRIORITY				(9)

#define		USE_SIIC0			/* Use SCI0 */
#define		USE_SSCL0_P21			/* Use SCI0 SSCL0 is P21 */
//#define	USE_SSCL0_P33			/* Use SCI0 SSCL0 is P33 */
#define		USE_SSDA0_P20			/* Use SCI0 SSDA0 is P20 */
//#define	USE_SSDA0_P32			/* Use SCI0 SSDA0 is P32 */
#define		SIIC0_FSCL	400000		/* SIIC0 SCL Frequency(Hz) */

//#define	USE_SIIC1			/* Use SCI1 */
#define		USE_SSCL1_P15			/* Use SCI1 SSCL1 is P15 */
//#define	USE_SSCL1_P30			/* Use SCI1 SSCL1 is P30 */
//#define	USE_SSCL1_PF2			/* Use SCI1 SSCL1 is PF2 */
#define		USE_SSDA1_P16			/* Use SCI1 SSDA1 is P16 */
//#define	USE_SSDA1_P26			/* Use SCI1 SSDA1 is P26 */
//#define	USE_SSDA1_PF0			/* Use SCI1 SSDA1 is PF0 */
#define		SIIC1_FSCL	400000		/* SIIC1 SCL Frequency(Hz) */

//#define	USE_SIIC2			/* Use SCI2 */
#define		USE_SSCL2_P12			/* Use SCI2 SSCL2 is P12 */
//#define	USE_SSCL2_P52			/* Use SCI2 SSCL2 is P52 */
#define		USE_SSDA2_P13			/* Use SCI2 SSDA2 is P13 */
//#define	USE_SSDA2_P50			/* Use SCI2 SSDA2 is P50 */
#define		SIIC2_FSCL	400000		/* SIIC2 SCL Frequency(Hz) */

//#define	USE_SIIC3			/* Use SCI3 */
#define		USE_SSCL3_P16			/* Use SCI3 SSCL3 is P16 */
//#define	USE_SSCL3_P25			/* Use SCI3 SSCL3 is P25 */
#define		USE_SSDA3_P17			/* Use SCI3 SSDA3 is P17 */
//#define	USE_SSDA3_P23			/* Use SCI3 SSDA3 is P23 */
#define		SIIC3_FSCL	400000		/* SIIC3 SCL Frequency(Hz) */

//#define	USE_SIIC4			/* Use SCI4 */
#define		USE_SSCL4_PB0			/* Use SCI4 SSCL4 is PB0 */
#define		USE_SSDA4_PB1			/* Use SCI4 SSDA4 is PB1 */
#define		SIIC4_FSCL	400000		/* SIIC4 SCL Frequency(Hz) */

//#define	USE_SIIC5			/* Use SCI5 */
#define		USE_SSCL5_PA2			/* Use SCI5 SSCL5 is PA2 */
//#define	USE_SSCL5_PA3			/* Use SCI5 SSCL5 is PA3 */
//#define	USE_SSCL5_PC2			/* Use SCI5 SSCL5 is PC2 */
#define		USE_SSDA5_PA4			/* Use SCI5 SSDA5 is PA4 */
//#define	USE_SSDA5_PC3			/* Use SCI5 SSDA5 is PC3 */
#define		SIIC5_FSCL	400000		/* SIIC5 SCL Frequency(Hz) */

//#define	USE_SIIC6			/* Use SCI6 */
#define		USE_SSCL6_P01			/* Use SCI6 SSCL6 is P01 */
//#define	USE_SSCL6_P33			/* Use SCI6 SSCL6 is P33 */
//#define	USE_SSCL6_PB0			/* Use SCI6 SSCL6 is PB0 */
#define		USE_SSDA6_P00			/* Use SCI6 SSDA6 is P00 */
//#define	USE_SSDA6_P32			/* Use SCI6 SSDA6 is P32 */
//#define	USE_SSDA6_PB1			/* Use SCI6 SSDA6 is PB1 */
#define		SIIC6_FSCL	400000		/* SIIC6 SCL Frequency(Hz) */

//#define	USE_SIIC7			/* Use SCI7 */
#define		USE_SSCL7_P57			/* Use SCI7 SSCL7 is P57 */
//#define	USE_SSCL7_P92			/* Use SCI7 SSCL7 is P92 */
//#define	USE_SSCL7_PH1			/* Use SCI7 SSCL7 is PH1 */
#define		USE_SSDA7_P55			/* Use SCI7 SSDA7 is P55 */
//#define	USE_SSDA7_P90			/* Use SCI7 SSDA7 is P90 */
//#define	USE_SSDA7_PH2			/* Use SCI7 SSDA7 is PH2 */
#define		SIIC7_FSCL	400000		/* SIIC7 SCL Frequency(Hz) */

//#define	USE_SIIC8			/* Use SCI8 */
#define		USE_SSCL8_PC6			/* Use SCI8 SSCL8 is PC6 */
//#define	USE_SSCL8_PJ1			/* Use SCI8 SSCL8 is PJ1 */
//#define	USE_SSCL8_PK1			/* Use SCI8 SSCL8 is PK1 */
#define		USE_SSDA8_PC7			/* Use SCI8 SSDA8 is PC7 */
//#define	USE_SSDA8_PJ2			/* Use SCI8 SSDA8 is PJ2 */
//#define	USE_SSDA8_PK2			/* Use SCI8 SSDA8 is PK2 */
#define		SIIC8_FSCL	400000		/* SIIC8 SCL Frequency(Hz) */

//#define	USE_SIIC9			/* Use SCI9 */
#define		USE_SSCL9_PB6			/* Use SCI9 SSCL9 is PB6 */
//#define	USE_SSCL9_PL1			/* Use SCI9 SSCL9 is PL1 */
#define		USE_SSDA9_PB7			/* Use SCI9 SSDA9 is PB7 */
//#define	USE_SSDA9_PL2			/* Use SCI9 SSDA9 is PL2 */
#define		SIIC9_FSCL	400000		/* SIIC9 SCL Frequency(Hz) */

//#define	USE_SIIC10			/* Use SCI10 */
#define		USE_SSCL10_P81			/* Use SCI10 SSCL10 is P81 */
//#define	USE_SSCL10_P86			/* Use SCI10 SSCL10 is P86 */
//#define	USE_SSCL10_PC6			/* Use SCI10 SSCL10 is PC6 */
//#define	USE_SSCL10_PM1			/* Use SCI10 SSCL10 is PM1 */
#define		USE_SSDA10_P82			/* Use SCI10 SSDA10 is P82 */
//#define	USE_SSDA10_P87			/* Use SCI10 SSDA10 is P87 */
//#define	USE_SSDA10_PC7			/* Use SCI10 SSDA10 is PC7 */
//#define	USE_SSDA10_PM2			/* Use SCI10 SSDA10 is PM2 */
#define		SIIC10_FSCL	400000		/* SIIC10 SCL Frequency(Hz) */

//#define	USE_SIIC11			/* Use SCI11 */
#define		USE_SSCL11_P76			/* Use SCI11 SSCL11 is P76 */
//#define	USE_SSCL11_PB6			/* Use SCI11 SSCL11 is PB6 */
//#define	USE_SSCL11_PQ1			/* Use SCI11 SSCL11 is PQ1 */
#define		USE_SSDA11_P77			/* Use SCI11 SSDA11 is P77 */
//#define	USE_SSDA11_PB7			/* Use SCI11 SSDA11 is PB7 */
//#define	USE_SSDA11_PQ2			/* Use SCI11 SSDA11 is PQ2 */
#define		SIIC11_FSCL	400000		/* SIIC11 SCL Frequency(Hz) */

//#define	USE_SIIC12			/* Use SCI12 */
#define		USE_SSCL12_PE2			/* Use SCI12 SSCL12 is PE2 */
#define		USE_SSDA12_PE1			/* Use SCI12 SSDA12 is PE1 */
#define		SIIC12_FSCL	400000		/* SIIC12 SCL Frequency(Hz) */

#endif	/* SIIC_CONFIG_H */