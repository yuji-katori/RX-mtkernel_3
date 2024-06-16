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
#define		USE_TXD0_P20			/* Use SCI0 TXD0 is P20 */

#endif	/* SCI_CONFIG_H */