/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2022/12/31.
 *----------------------------------------------------------------------
 */

/*
 *	config_tcpudp.h
 */
#ifndef CONFIG_TCPUDP_H
#define CONFIG_TCPUDP_H

/* Ether task priority. */
#define	ETHER_CFG_TASK_PRIORITY				(3)

/* EINT interrupt priority level. */
#define	ETHER_CFG_INT_PRIORITY				(8)

#define	CFG_SYSTEM_DHCP					(1)
#define	CFG_FIXED_IP_ADDRESS_CH0			192,168,1,200
#define	CFG_FIXED_SABNET_MASK_CH0			255,255,255,0
#define	CFG_FIXED_GATEWAY_ADDRESS_CH0			192,168,1,1
#define	CFG_ETHER_CH0_MAC_ADDRESS			0x00,0x00,0x00,0x00,0x00,0x02
#define	CFG_FIXED_IP_ADDRESS_CH1			192,168,0,150
#define	CFG_FIXED_SABNET_MASK_CH1			255,255,255,0
#define	CFG_FIXED_GATEWAY_ADDRESS_CH1			192,168,0,1
#define	CFG_ETHER_CH1_MAC_ADDRESS			0x00,0x00,0x00,0x00,0x00,0x04
#define	CFG_TCP_REPID_NUM				(1)
#define	CFG_TCP_REPID1_PORT_NUMBER			(1024)
#define	CFG_TCP_REPID2_PORT_NUMBER			(1025)
#define	CFG_TCP_REPID3_PORT_NUMBER			(1026)
#define	CFG_TCP_REPID4_PORT_NUMBER			(1027)
#define	CFG_TCP_CEPID_NUM				(1)
#define	CFG_TCP_CEPID1_CHANNEL				(0)
#define	CFG_TCP_CEPID1_RECEIVE_WINDOW_SIZE		(1460)
#define	CFG_TCP_CEPID1_KEEPALIVE_ENABLE			(0)
#define	CFG_TCP_CEPID2_CHANNEL				(0)
#define	CFG_TCP_CEPID2_RECEIVE_WINDOW_SIZE		(1460)
#define	CFG_TCP_CEPID2_KEEPALIVE_ENABLE			(0)
#define	CFG_TCP_CEPID3_CHANNEL				(0)
#define	CFG_TCP_CEPID3_RECEIVE_WINDOW_SIZE		(1460)
#define	CFG_TCP_CEPID3_KEEPALIVE_ENABLE			(0)
#define	CFG_TCP_CEPID4_CHANNEL				(0)
#define	CFG_TCP_CEPID4_RECEIVE_WINDOW_SIZE		(1460)
#define	CFG_TCP_CEPID4_KEEPALIVE_ENABLE			(0)
#define	CFG_TCP_CEPID5_CHANNEL				(0)
#define	CFG_TCP_CEPID5_RECEIVE_WINDOW_SIZE		(1460)
#define	CFG_TCP_CEPID5_KEEPALIVE_ENABLE			(0)
#define	CFG_TCP_CEPID6_CHANNEL				(0)
#define	CFG_TCP_CEPID6_RECEIVE_WINDOW_SIZE		(1460)
#define	CFG_TCP_CEPID6_KEEPALIVE_ENABLE			(0)
#define	CFG_TCP_MSS					(1460)
#define	CFG_TCP_2MSL_TIME				(60)
#define	CFG_TCP_MAX_TIMEOUT_PERIOD			(600)
#define	CFG_TCP_DIVIDE_SENDING_PACKET			(1)
#define	CFG_TCP_KEEPALIVE_START				(7200)
#define	CFG_TCP_KEEPALIVE_INTERVAL			(10)
#define	CFG_TCP_KEEPALIVE_COUNT				(10)
#define	CFG_UDP_CEPID_NUM				(1)
#define	CFG_UDP_CEPID1_CHANNEL				(0)
#define	CFG_UDP_CEPID1_PORT_NUMBER			(1365)
#define	CFG_UDP_CEPID2_CHANNEL				(0)
#define	CFG_UDP_CEPID2_PORT_NUMBER			(1366)
#define	CFG_UDP_CEPID3_CHANNEL				(0)
#define	CFG_UDP_CEPID3_PORT_NUMBER			(1367)
#define	CFG_UDP_CEPID4_CHANNEL				(0)
#define	CFG_UDP_CEPID4_PORT_NUMBER			(1368)
#define	CFG_UDP_CEPID5_CHANNEL				(0)
#define	CFG_UDP_CEPID5_PORT_NUMBER			(1369)
#define	CFG_UDP_CEPID6_CHANNEL				(0)
#define	CFG_UDP_CEPID6_PORT_NUMBER			(1370)
#define	CFG_UDP_MULTICAST_TTL				(1)
#define	CFG_UDP_BEHAVIOR_OF_RECEIVED_ZERO_CHECKSUM	(0)
#define	CFG_IP_ARP_CACHE_TABLE_COUNT			(3)

#endif	/* CONFIG_TCPUDP_H */