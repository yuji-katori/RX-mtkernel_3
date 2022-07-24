/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	ether_driver.h
 */

typedef struct
{
	/* t4 status */
	UW	t4_rec_cnt;
	UW	t4_rec_byte;
	UW	t4_snd_cnt;
	UW	t4_snd_byte;

	/* report error */
	UW	re_len_cnt;
	UW	re_network_layer_cnt;
	UW	re_transport_layer_cnt;
	UW	re_arp_header1_cnt;
	UW	re_arp_header2_cnt;
	UW	re_ip_header1_cnt;
	UW	re_ip_header2_cnt;
	UW	re_ip_header3_cnt;
	UW	re_ip_header4_cnt;
	UW	re_ip_header5_cnt;
	UW	re_ip_header6_cnt;
	UW	re_ip_header7_cnt;
	UW	re_ip_header8_cnt;
	UW	re_ip_header9_cnt;
	UW	re_tcp_header1_cnt;
	UW	re_tcp_header2_cnt;
	UW	re_udp_header1_cnt;
	UW	re_udp_header2_cnt;
	UW	re_udp_header3_cnt;
	UW	re_icmp_header1_cnt;
	UW	re_igmp_header1_cnt;
	UW	re_igmp_header2_cnt;
	UW	re_dhcp_header1_cnt;
	UW	re_dhcp_header2_cnt;
} T4_STATISTICS;

#define	RE_LEN			(-1)
#define	RE_NETWORK_LAYER	(-2)
#define	RE_TRANSPORT_LAYER	(-3)
#define	RE_ARP_HEADER1		(-21)
#define	RE_ARP_HEADER2		(-22)
#define	RE_IP_HEADER1		(-41)
#define	RE_IP_HEADER2		(-42)
#define	RE_IP_HEADER3		(-43)
#define	RE_IP_HEADER4		(-44)
#define	RE_IP_HEADER5		(-45)
#define	RE_IP_HEADER6		(-46)
#define	RE_IP_HEADER7		(-47)
#define	RE_IP_HEADER8		(-48)
#define	RE_IP_HEADER9		(-49)
#define	RE_TCP_HEADER1		(-61)
#define	RE_TCP_HEADER2		(-62)
#define	RE_UDP_HEADER1		(-81)
#define	RE_UDP_HEADER2		(-82)
#define	RE_UDP_HEADER3		(-83)
#define	RE_ICMP_HEADER1		(-101)
#define	RE_IGMP_HEADER1		(-121)
#define	RE_IGMP_HEADER2		(-122)
#define	RE_DHCP_ILLEGAL		(-131)
#define	RE_DHCP_SND_TIMEOUT	(-132)

void _process_tcpip(void);