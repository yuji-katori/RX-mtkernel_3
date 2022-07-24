/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	dev_ether.h
 */
#ifndef DEV_ETHER_HEADER_FILE
#define DEV_ETHER_HEADER_FILE

#include <tk/tkernel.h>
#include "../ether/ether_config.h"

/*** IP address/Port No. information ***/
typedef struct
{
	UW		ipaddr;		/* IP address */
	UH		portno;		/* Port number */
} T_IPV4EP;

/***  TCP reception point  ***/
typedef struct
{
	ATR		repatr;		/* TCP reception point attribute    */
	T_IPV4EP	myaddr;		/* Local IP address and port number */
} T_TCP_CREP;

/***  TCP communication end point  ***/
typedef struct
{
	ATR		cepatr;			/* TCP communication end point attribute  */
	void*		sbuf;			/* Top address of transmit window buffer  */
	INT		sbufsz;			/* Size of transmit window buffer         */
	void*		rbuf;			/* Top address of receive window buffer   */
	INT		rbufsz;			/* Size of receive window buffer          */
	ER		(*callback)(ID cepid, FN fncd , void *p_parblk);	/* Callback routine */
	UW		lan_port_number;	/* LAN port number */
	UW		keepalive_enable;	/* Keep-alive function */
} T_TCP_CCEP;

/***  UDP communication end point  ***/
typedef struct
{
	ATR		cepatr;			/* UDP communication end point attribute  */
	T_IPV4EP	myaddr;			/* Local IP address and port number       */
	ER		(*callback)(ID cepid, FN fncd , void *p_parblk);	/* Callback routine */
	UW		lan_port_number;	/* LAN port number */
} T_UDP_CCEP;

/***  IP address settings  ***/
typedef struct
{
	UB		ipaddr[4];	/* Local IP address        */
	UB		maskaddr[4];	/* Subnet mask             */
	UB		gwaddr[4];	/* Gateway address X       */
} TCPUDP_ENV;

typedef struct
{
	UB	ipaddr[4];
	UB	maskaddr[4];
	UB	gwaddr[4];
	UB	dnsaddr[4];
	UB	dnsaddr2[4];
	VB	domain[253+1];
	UB	macaddr[6];
} DHCP;

IMPORT ER lan_open(void);
IMPORT ER lan_close(void);
IMPORT ER tcpudp_open(UW *workp);
IMPORT ER tcpudp_close(void);
IMPORT ER tcpudp_reset(UB channel);
IMPORT W  tcpudp_get_ramsize(void);

IMPORT ER tcp_acp_cep(ID cepid, ID repid, T_IPV4EP *p_dstadr, TMO tmout);
IMPORT ER tcp_con_cep(ID cepid, T_IPV4EP *p_myadr, T_IPV4EP *p_dstadr, TMO tmout);
IMPORT ER tcp_sht_cep(ID cepid);
IMPORT ER tcp_cls_cep(ID cepid, TMO tmout);
IMPORT ER tcp_snd_dat(ID cepid, void *data, INT dlen, TMO tmout);
IMPORT ER tcp_rcv_dat(ID cepid, void *data, INT dlen, TMO tmout);

IMPORT ER udp_snd_dat(ID cepid, T_IPV4EP *p_dstaddr, void *data, INT len, TMO tmout);
IMPORT ER udp_rcv_dat(ID cepid, T_IPV4EP *p_dstaddr, void *data, INT len, TMO tmout);

IMPORT const UB _t4_channel_num;
IMPORT const UB _t4_dhcp_enable;
IMPORT T_TCP_CREP tcp_crep[];
IMPORT T_TCP_CCEP tcp_ccep[];
IMPORT T_UDP_CCEP udp_ccep[];
IMPORT const H __tcprepn;
IMPORT const H __tcpcepn;
IMPORT const H __udpcepn;

#define	NADR		0		// IP address, port number specification omitted
#define	IPV4_ADDRANY	0		// IP address specification omitted
#define	TCP_PORTANY	0		// TCP port number specification omitted
#define	UDP_PORTANY	0		// UDP port number specification omitted

#define	E_CLS		-52		// Connection failure
#define	E_BOVR		-58		// Buffer overflow

#endif	/* DEV_ETHER_HEADER_FILE */