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

#define		USE_IIC0			/* Use RIIC0 */
#define		USE_SCL_P16			/* Use RIIC0 SCL is P16 */
//#define	USE_SCL_P12			/* Use RIIC0 SCL is P12 */
#define		USE_SDA_P17			/* Use RIIC0 SDA is P17 */
//#define	USE_SDA_P13			/* Use RIIC0 SDA is P13 */
#define		IIC0_HPSCL	0.6F		/* RIIC0 I2C High Period of the SCL Clock(us) */
#define		IIC0_LPSCL	1.3F		/* RIIC0 I2C Low  Period of the SCL Clock(us) */

#endif	/* IIC_CONFIG_H */