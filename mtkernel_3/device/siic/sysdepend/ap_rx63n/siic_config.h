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

#define	USE_SIIC2			/* Use SCI2 */
#define		USE_SSCL2_P12			/* Use SCI2 SSCL2 is P12 */
#define		USE_SSDA2_P13			/* Use SCI2 SSDA2 is P13 */
#define		SIIC2_FSCL	400000		/* SIIC2 SCL Frequency(Hz) */

#endif	/* SIIC_CONFIG_H */