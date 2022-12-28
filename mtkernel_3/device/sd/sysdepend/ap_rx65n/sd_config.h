/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	sd_config.h
 */
#ifndef SD_CONFIG_H
#define SD_CONFIG_H

/* SD control task priority. */
#define	SD_CFG_TASK_PRIORTY				(3)

/* SD interrupt priority level. */
#define	SD_CFG_INT_PRIORTY				(8)

/* SD DMA channel. */
#define	SD_CFG_DMA_CHANNEL				(4)

#endif	/* SD_CONFIG_H */