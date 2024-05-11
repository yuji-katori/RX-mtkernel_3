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

#define		USE_SIIC1			/* Use SCI1 */
#define		USE_SSCL1_P15			/* Use SCI1 SSCL1 is P15 */
//#define	USE_SSCL1_P30			/* Use SCI1 SSCL1 is P30 */
#define		USE_SSDA1_P16			/* Use SCI1 SSDA1 is P16 */
//#define	USE_SSDA1_P26			/* Use SCI1 SSDA1 is P26 */
#define		SIIC1_FSCL	400000		/* SIIC1 SCL Frequency(Hz) */

//#define	USE_SIIC5			/* Use SCI5 */
#define		USE_SSCL5_PC2			/* Use SCI5 SSCL5 is PC2 */
#define		USE_SSDA5_PC3			/* Use SCI5 SSDA5 is PC3 */
#define		SIIC5_FSCL	400000		/* SIIC5 SCL Frequency(Hz) */

#endif	/* SIIC_CONFIG_H */