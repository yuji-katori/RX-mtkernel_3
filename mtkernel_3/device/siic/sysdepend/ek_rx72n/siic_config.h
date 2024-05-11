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

#define		USE_SIIC11			/* Use SCI11 */
#define		USE_SSCL11_PB6			/* Use SCI11 SSCL11 is PB6 */
#define		USE_SSDA11_PB7			/* Use SCI11 SSDA11 is PB7 */
#define		SIIC11_FSCL	400000		/* SIIC11 SCL Frequency(Hz) */

#endif	/* SIIC_CONFIG_H */