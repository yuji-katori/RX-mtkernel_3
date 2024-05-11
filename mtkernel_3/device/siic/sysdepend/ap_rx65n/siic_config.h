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
#define		USE_SSDA0_P20			/* Use SCI0 SSDA0 is P20 */
#define		SIIC0_FSCL	400000		/* SIIC0 SCL Frequency(Hz) */

#endif	/* SIIC_CONFIG_H */