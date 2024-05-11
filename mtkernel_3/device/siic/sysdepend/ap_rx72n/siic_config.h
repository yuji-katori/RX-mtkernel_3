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
#define		USE_SSCL1_P30			/* Use SCI1 SSCL1 is P30 */
#define		USE_SSDA1_P26			/* Use SCI1 SSDA1 is P26 */
#define		SIIC1_FSCL	400000		/* SIIC1 SCL Frequency(Hz) */

//#define	USE_SIIC3			/* Use SCI3 */
#define		USE_SSCL3_P25			/* Use SCI3 SSCL3 is P25 */
#define		USE_SSDA3_P23			/* Use SCI3 SSDA3 is P23 */
#define		SIIC3_FSCL	400000		/* SIIC3 SCL Frequency(Hz) */

//#define	USE_SIIC6			/* Use SCI6 */
#define		USE_SSCL6_P01			/* Use SCI6 SSCL6 is P01 */
//#define	USE_SSCL6_P33			/* Use SCI6 SSCL6 is P33 */
#define		USE_SSDA6_P00			/* Use SCI6 SSDA6 is P00 */
//#define	USE_SSDA6_P32			/* Use SCI6 SSDA6 is P32 */
#define		SIIC6_FSCL	400000		/* SIIC6 SCL Frequency(Hz) */

//#define	USE_SIIC11			/* Use SCI11 */
#define		USE_SSCL11_P76			/* Use SCI11 SSCL11 is P76 */
#define		USE_SSDA11_P77			/* Use SCI11 SSDA11 is P77 */
#define		SIIC11_FSCL	400000		/* SIIC11 SCL Frequency(Hz) */

#endif	/* SIIC_CONFIG_H */