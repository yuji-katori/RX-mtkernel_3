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

#define	USE_SIIC1			/* Use SCI1 */
#define		USE_SSCL1_P15			/* Use SCI1 SSCL1 is P15 */
//#define	USE_SSCL1_P30			/* Use SCI1 SSCL1 is P30 */
#define		USE_SSDA1_P16			/* Use SCI1 SSDA1 is P16 */
//#define	USE_SSDA1_P26			/* Use SCI1 SSDA1 is P26 */
#define		SIIC1_FSCL	400000		/* SIIC1 SCL Frequency(Hz) */

#define	USE_SIIC5			/* Use SCI5 */
#define		USE_SSCL5_PC2			/* Use SCI5 SSCL5 is PC2 */
#define		USE_SSDA5_PC3			/* Use SCI5 SSDA5 is PC3 */
#define		SIIC5_FSCL	400000		/* SIIC5 SCL Frequency(Hz) */

#define	USE_SIIC8			/* Use SCI8 */
#define		USE_SSCL8_PC6			/* Use SCI8 SSCL8 is PC6 */
#define		USE_SSDA8_PC7			/* Use SCI8 SSDA8 is PC7 */
#define		SIIC8_FSCL	400000		/* SIIC8 SCL Frequency(Hz) */

#define	USE_SIIC12			/* Use SCI12 */
#define		USE_SSCL12_PE2			/* Use SCI12 SSCL12 is PE2 */
#define		USE_SSDA12_PE1			/* Use SCI12 SSDA12 is PE1 */
#define		SIIC12_FSCL	400000		/* SIIC12 SCL Frequency(Hz) */

#endif	/* SIIC_CONFIG_H */