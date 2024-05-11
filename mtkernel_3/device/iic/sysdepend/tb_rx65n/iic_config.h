/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	iic_config.h
 */
#ifndef IIC_CONFIG_H
#define IIC_CONFIG_H

/* IIC control task priority. */
#define	IIC_CFG_TASK_PRIORITY				(4)

/* IIC interrupt priority level. */
#define	IIC_CFG_INT_PRIORITY				(9)

#define	USE_IIC0			/* Use RIIC0 */
#define	IIC0_HPSCL	0.6F		/* RIIC0 I2C High Period of the SCL Clock(us) */
#define	IIC0_LPSCL	1.3F		/* RIIC0 I2C Low  Period of the SCL Clock(us) */

#define	USE_IIC1			/* Use RIIC1 */
#define	IIC1_HPSCL	0.6F		/* RIIC1 I2C High Period of the SCL Clock(us) */
#define	IIC1_LPSCL	1.3F		/* RIIC1 I2C Low  Period of the SCL Clock(us) */

#define	USE_IIC2			/* Use RIIC2 */
#define	IIC2_HPSCL	0.6F		/* RIIC2 I2C High Period of the SCL Clock(us) */
#define	IIC2_LPSCL	1.3F		/* RIIC2 I2C Low  Period of the SCL Clock(us) */

#endif	/* IIC_CONFIG_H */