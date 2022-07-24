/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	config_tcpudp.c
 */
#include <stddef.h>
#include "r_t4_itcpip.h"
#include "ether_config.h"
#include "config/config_tcpudp.h"

const UB _t4_channel_num = CFG_SYSTEM_CHANNEL_NUMBER;
const UB _t4_dhcp_enable = CFG_SYSTEM_DHCP;
const UH _t4_dhcp_ip_reply_arp_delay = 300;

const T_TCP_CREP tcp_crep[] =
{	/* { attribute of reception point, {local IP address, local port number}} */
#if CFG_TCP_REPID_NUM >= 1
    { 0, { 0, CFG_TCP_REPID1_PORT_NUMBER }},
#if CFG_TCP_REPID_NUM >= 2
    { 0, { 0, CFG_TCP_REPID2_PORT_NUMBER }},
#if CFG_TCP_REPID_NUM >= 3
    { 0, { 0, CFG_TCP_REPID3_PORT_NUMBER }},
#if CFG_TCP_REPID_NUM >= 4
    { 0, { 0, CFG_TCP_REPID4_PORT_NUMBER }},
#endif
#endif
#endif
#endif
};

/* Total number of TCP reception points */
const H __tcprepn = sizeof(tcp_crep) / sizeof(T_TCP_CREP);

/***  Definition of TCP communication end point
      (only receive window size needs to be set) ***/
const T_TCP_CCEP tcp_ccep[] =
{	/* {	attribute of TCP communication end point,
		top address of transmit window buffer, size of transmit window buffer,
		top address of receive window buffer, size of receive window buffer,
		address of callback routine,
		channel number of ether port, use'd keepalive function } */
#if CFG_TCP_CEPID_NUM >= 1
	{	0, 0, 0, 0, CFG_TCP_CEPID1_RECEIVE_WINDOW_SIZE, NULL,
		CFG_TCP_CEPID1_CHANNEL, CFG_TCP_CEPID1_KEEPALIVE_ENABLE	},
#if CFG_TCP_CEPID_NUM >= 2
	{	0, 0, 0, 0, CFG_TCP_CEPID2_RECEIVE_WINDOW_SIZE, NULL,
		CFG_TCP_CEPID2_CHANNEL, CFG_TCP_CEPID2_KEEPALIVE_ENABLE	},
#if CFG_TCP_CEPID_NUM >= 3
	{	0, 0, 0, 0, CFG_TCP_CEPID3_RECEIVE_WINDOW_SIZE, NULL,
		CFG_TCP_CEPID3_CHANNEL, CFG_TCP_CEPID3_KEEPALIVE_ENABLE	},
#if CFG_TCP_CEPID_NUM >= 4
	{	0, 0, 0, 0, CFG_TCP_CEPID4_RECEIVE_WINDOW_SIZE, NULL,
		CFG_TCP_CEPID4_CHANNEL, CFG_TCP_CEPID4_KEEPALIVE_ENABLE	},
#if CFG_TCP_CEPID_NUM >= 5
	{	0, 0, 0, 0, CFG_TCP_CEPID5_RECEIVE_WINDOW_SIZE, NULL,
		CFG_TCP_CEPID5_CHANNEL, CFG_TCP_CEPID5_KEEPALIVE_ENABLE	},
#if CFG_TCP_CEPID_NUM >= 6
	{	0, 0, 0, 0, CFG_TCP_CEPID6_RECEIVE_WINDOW_SIZE, NULL,
		CFG_TCP_CEPID6_CHANNEL, CFG_TCP_CEPID6_KEEPALIVE_ENABLE	},
#endif
#endif
#endif
#endif
#endif
#endif
};

/* Total number of TCP communication end points */
const H __tcpcepn = sizeof(tcp_ccep) / sizeof(T_TCP_CCEP);

/***  TCP MSS  ***/
const UH _tcp_mss[] =
{	/* MAX:1460 bytes */
	CFG_TCP_MSS,
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	CFG_TCP_MSS,
#endif
};

/***  2MSL wait time (unit:10ms)  ***/
const UH    _tcp_2msl[] =
{
	(CFG_TCP_2MSL_TIME * (1000 / 10)),		/* 1 min */
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	(CFG_TCP_2MSL_TIME * (1000 / 10)),		/* 1 min */
#endif
};
/***  Maximum value of retransmission timeout period (unit:10ms)  ***/
const UH    _tcp_rt_tmo_rst[] =
{
	(CFG_TCP_MAX_TIMEOUT_PERIOD * (1000 / 10)),	/* 10 min */
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	(CFG_TCP_MAX_TIMEOUT_PERIOD * (1000 / 10)),	/* 10 min */
#endif
};

/***  Transmit for delay ack (ON=1/OFF=0) ***/
const UB   _tcp_dack[] =
{
	CFG_TCP_DIVIDE_SENDING_PACKET,
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	CFG_TCP_DIVIDE_SENDING_PACKET,
#endif
};

/***  Time to first transmit Keepalive packet (unit:10ms)  ***/
const UW   _tcp_keepalive_start[] =
{
	(CFG_TCP_KEEPALIVE_START * (1000 / 10)),
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	(CFG_TCP_KEEPALIVE_START * (1000 / 10)),
#endif
};

/***  Second Keepalive packets transmission interval (unit:10ms)  ***/
const UW   _tcp_keepalive_interval[] =
{
	(CFG_TCP_KEEPALIVE_INTERVAL * (1000 / 10)),
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	(CFG_TCP_KEEPALIVE_INTERVAL * (1000 / 10)),
#endif
};

/***  Keepalive packet transmission count  ***/
const UW   _tcp_keepalive_count[] =
{
	CFG_TCP_KEEPALIVE_COUNT,
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	CFG_TCP_KEEPALIVE_COUNT,
#endif
};

/****************************************************************************/
/**********************     UDP-related definition     **********************/
/****************************************************************************/
/***  Definition of UDP communication end point  ***/
const T_UDP_CCEP udp_ccep[] =
{
#if CFG_UDP_CEPID_NUM >= 1
    { 0, { 0, CFG_UDP_CEPID1_PORT_NUMBER }, NULL, CFG_UDP_CEPID1_CHANNEL },
#if CFG_UDP_CEPID_NUM >= 2
    { 0, { 0, CFG_UDP_CEPID2_PORT_NUMBER }, NULL, CFG_UDP_CEPID2_CHANNEL },
#if CFG_UDP_CEPID_NUM >= 3
    { 0, { 0, CFG_UDP_CEPID3_PORT_NUMBER }, NULL, CFG_UDP_CEPID3_CHANNEL },
#if CFG_UDP_CEPID_NUM >= 4
    { 0, { 0, CFG_UDP_CEPID4_PORT_NUMBER }, NULL, CFG_UDP_CEPID4_CHANNEL },
#if CFG_UDP_CEPID_NUM >= 5
    { 0, { 0, CFG_UDP_CEPID5_PORT_NUMBER }, NULL, CFG_UDP_CEPID5_CHANNEL },
#if CFG_UDP_CEPID_NUM >= 6
    { 0, { 0, CFG_UDP_CEPID6_PORT_NUMBER }, NULL, CFG_UDP_CEPID6_CHANNEL },
#endif
#endif
#endif
#endif
#endif
#endif
};
/* Total number of UDP communication end points */
const H __udpcepn = (sizeof(udp_ccep) / sizeof(T_UDP_CCEP));

/***  TTL for multicast transmission  ***/
const UB __multi_TTL[] = 
{
	CFG_UDP_MULTICAST_TTL,
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	CFG_UDP_MULTICAST_TTL,
#endif
};

/*** Behavior of UDP zero checksum ***/
const UB _udp_enable_zerochecksum[] =
{
	CFG_UDP_BEHAVIOR_OF_RECEIVED_ZERO_CHECKSUM,
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	CFG_UDP_BEHAVIOR_OF_RECEIVED_ZERO_CHECKSUM,
#endif
}; /* 0 = disable, other = enable */

/****************************************************************************/
/**********************  SYSTEM-callback definition   ***********************/
/****************************************************************************/
const callback_from_system_t g_fp_user = CFG_SYSTEM_CALLBACK_FUNCTION_NAME;

/****************************************************************************/
/**********************     IP-related definition     ***********************/
/****************************************************************************/
const UH _ip_tblcnt[] =
{
	CFG_IP_ARP_CACHE_TABLE_COUNT,
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	CFG_IP_ARP_CACHE_TABLE_COUNT,
#endif
};

#define	MY_IP_ADDR0	CFG_FIXED_IP_ADDRESS_CH0		/* Local IP address  */
#define	GATEWAY_ADDR0	CFG_FIXED_GATEWAY_ADDRESS_CH0		/* Gateway address (invalid if all 0s) */
#define	SUBNET_MASK0	CFG_FIXED_SABNET_MASK_CH0		/* Subnet mask  */

#if CFG_SYSTEM_CHANNEL_NUMBER == 2
#define	MY_IP_ADDR1	CFG_FIXED_IP_ADDRESS_CH1		/* Local IP address  */
#define	GATEWAY_ADDR1	CFG_FIXED_GATEWAY_ADDRESS_CH1		/* Gateway address (invalid if all 0s) */
#define	SUBNET_MASK1	CFG_FIXED_SABNET_MASK_CH1		/* Subnet mask  */
#endif

TCPUDP_ENV tcpudp_env[] =
{
	{ {MY_IP_ADDR0 }, { SUBNET_MASK0 }, { GATEWAY_ADDR0 } },
#if T4_CFG_SYSTEM_CHANNEL_NUMBER == 2
	{ {MY_IP_ADDR1 }, { SUBNET_MASK1 }, { GATEWAY_ADDR1 } },
#endif
};

/****************************************************************************/
/**********************     Driver-related definition     *******************/
/****************************************************************************/
/*--------------------------------------------------------------------------*/
/*    Set of Ethernet-related                                               */
/*--------------------------------------------------------------------------*/
/* Local MAC address (Set all 0s when unspecified) */
#define	MY_MAC_ADDR0	CFG_ETHER_CH0_MAC_ADDRESS

#if CFG_SYSTEM_CHANNEL_NUMBER == 2
#define	MY_MAC_ADDR1	CFG_ETHER_CH1_MAC_ADDRESS
#endif

UB _myethaddr[][6] =
{
	{ MY_MAC_ADDR0 },
#if CFG_SYSTEM_CHANNEL_NUMBER == 2
	{ MY_MAC_ADDR1 },
#endif
};