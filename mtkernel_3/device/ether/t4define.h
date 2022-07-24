/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	t4define.h
 */
#ifndef _T4DEFINE_H_
#define _T4DEFINE_H_

#ifdef PRINTDEBUG
#define _STDIO
#ifdef CLANGSPEC
int  tm_printf( const char *format, ... );
#else
int  tm_printf( const unsigned char *format, ... );
#endif /* CLANGSPEC */
#endif /* PRINTDEBUG */

#ifdef	__RXV3
#define	__RXV2
#endif

#define _TCP
#define _UDP
#define _ICMP
#define _IGMP
#define _MULTI
#define _TCP_DACK
#define _TEST_LIBRARY
#define _ETHER
/*
#define lan_open		LAN_OPEN
#define lan_close		LAN_CLOSE
#define tcpudp_open		TCPUDP_OPEN
#define tcpudp_close		TCPUDP_CLOSE
#define tcpudp_reset		TCPUDP_RESET
#define tcpudp_get_ramsize	TCPUDP_GET_RAMSIZE

#define udp_snd_dat		UDP_SND_DAT
#define udp_rcv_dat		UDP_RCV_DAT
#define udp_can_cep		UDP_CAN_CEP

#define tcp_acp_cep		TCP_ACP_CEP
#define tcp_con_cep		TCP_CON_CEP
#define tcp_sht_cep		TCP_SHT_CEP
#define tcp_cls_cep		TCP_CLS_CEP
#define tcp_snd_dat		TCP_SND_DAT
#define tcp_rcv_dat		TCP_RCV_DAT
#define tcp_can_cep		TCP_CAN_CEP
*/
#define	printf			tm_printf
/*
#ifdef _R_T4_ITCPIP_H
ER UDP_SND_DAT(ID cepid, T_IPV4EP *p_dstaddr, void *data, INT len, TMO tmout);
#endif	/* _R_T4_ITCPIP_H */

#endif	/* _T4DEFINE_H_ */