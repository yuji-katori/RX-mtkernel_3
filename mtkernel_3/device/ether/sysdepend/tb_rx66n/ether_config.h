/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2023/10/23.
 *    Modified by Yuji Katori at 2023/12/28.
 *----------------------------------------------------------------------
 */

/*
 *	ether_config.h
 */
#ifndef ETHER_CONFIG_H
#define ETHER_CONFIG_H

/* PMGI interrupt priority level. */
#define	ETHER_CFG_PMGI_INT_PRIORTY		ETHER_CFG_INT_PRIORITY

/* Please define the size of the sending and receiving buffer in the value where one frame can surely be stored 
   because the driver is single-frame/single-buffer processing.  */
#define	ETHER_CFG_BUFSIZE			(1536)		/* Must be 32-byte aligned */

#define	BSP_MCU_RX66N
#define	CFG_SYSTEM_CHANNEL_NUMBER		(1)
#define	CFG_SYSTEM_CALLBACK_FUNCTION_NAME	(system_callback)

/* Ethernet interface select.
 0 = MII  (Media Independent Interface)
 1 = RMII (Reduced Media Independent Interface) */
#define	ETHER_CFG_MODE_SEL			(1)

/* PHY-LSI address setting for ETHER0/1. */
#define	ETHER_CFG_CH0_PHY_ADDRESS		(1)		/* Please define the PHY-LSI address in the range of 0-31. */

/* The number of Rx descriptors. */
#define	ETHER_CFG_EMAC_RX_DESCRIPTORS		(CFG_TCP_CEPID_NUM+CFG_UDP_CEPID_NUM)

/* The number of Tx descriptors. */
#define	ETHER_CFG_EMAC_TX_DESCRIPTORS		(CFG_TCP_CEPID_NUM+CFG_UDP_CEPID_NUM)

/* The register bus of PHY0/1 for ETHER0/1 select
 0 = The access of the register of PHY uses ETHER0. */
#define	ETHER_CFG_CH0_PHY_ACCESS		(0)

/* Define the access timing of MII/RMII register */
#define	ETHER_CFG_PHY_MII_WAIT			(3)

/* Define the waiting time for reset completion of PHY-LSI */
#define	ETHER_CFG_PHY_DELAY_RESET		(0x00020000L)

/* Link status read from LMON bit of ETHERC PSR register.  The state is hardware dependent. */
#define	ETHER_CFG_LINK_PRESENT			(0)

/*  Use LINKSTA signal for detect link status changes */
#define	ETHER_CFG_USE_LINKSTA			(0)

/* Definition of whether or not to use KSZ8041NL of the Micrel Inc. */
#define	ETHER_CFG_USE_PHY_KSZ8041NL		(0)

/* Definition of whether or not to use non blocking of PHY Management Station Operation */
#define	ETHER_CFG_NON_BLOCKING			(0)

/* Define the clock of the PHY Management Station */
#define	ETHER_CFG_PMGI_CLOCK			(2500000)

/* PHY Management Station Preamble Control */
#define	ETHER_CFG_PMGI_ENABLE_PREAMBLE		(0)

/* Define the Hold Time Adjustment of the PHY Management Station */
#define	ETHER_CFG_PMGI_HOLD_TIME		(0)

/* Define the Capture Time Adjustment of the PHY Management Station */
#define	ETHER_CFG_PMGI_CAPTURE_TIME		(0)

#endif	/* ETHER_CONFIG_H */