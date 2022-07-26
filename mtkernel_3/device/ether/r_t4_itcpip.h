/***********************************************************************************************************************
* DISCLAIMER
* This software is supplied by Renesas Electronics Corporation and is only intended for use with Renesas products. No
* other uses are authorized. This software is owned by Renesas Electronics Corporation and is protected under all
* applicable laws, including copyright laws.
* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES REGARDING
* THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. ALL SUCH WARRANTIES ARE EXPRESSLY DISCLAIMED. TO THE MAXIMUM
* EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS ELECTRONICS CORPORATION NOR ANY OF ITS AFFILIATED COMPANIES
* SHALL BE LIABLE FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES FOR ANY REASON RELATED TO THIS
* SOFTWARE, EVEN IF RENESAS OR ITS AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
* Renesas reserves the right, without notice, to make changes to this software and to discontinue the availability of
* this software. By using this software, you agree to the additional terms and conditions found by accessing the
* following link:
* http://www.renesas.com/disclaimer
*
* Copyright (C) 2004-2021 Renesas Electronics Corporation, All Rights Reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : r_t4_itcpip.h
* Version      : 2.10
* Description  : TCP/IP library T4 Header file.
* Website      : https://www.renesas.com/mw/t4
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version  Description
*         : 25.08.2009 0.24     First beta Release from T3(since 2007) and T2(since 2004) code.
*         : 25.09.2009 0.25     Corresponded for RSK standard driver interface
*         : 08.10.2009 0.26     Applied T3 bug fix
*         : 11.03.2010 0.27     Fixed bug.
*         : 12.03.2010 0.28     Added setting variable "_tcp_dack"
*         : 27.08.2010 0.29     Corrected r_t4_itcpip.h
*         : 27.09.2010 1.00     First release
*         : 10.10.2010 1.01     Fixed bug.
*         : 05.01.2011 1.02     Fixed bug.
*         : 05.01.2011 1.03     Corrected r_t4_itcpip.h
*         : 23.08.2011 1.04     Added "report_error" function, cleanup code
*         : 01.04.2012 1.05     Added "PPP" connect and R8C support.
*         :                     Added SH-4A support.
*         :                     Added V850E2 support.
*         :                     Change user defined function spec
*         :                     api_slp() -> tcp_api_slp(), udp_api_slp(), ppp_api_slp()
*         :                     api_wup() -> tcp_api_wup(), udp_api_wup(), ppp_api_wup()
*         :                     Fixed bug.
*         : 21.06.2013 1.06     Fixed bug: r_t4_itcpip.h.
*         :                     Added UDP broadcast function.
*         :                     Fixed bug: TCP 3way hand-shake behavior with zero-window size SYN.
*         :                     Fixed bug: TCP sending.
*         :                     Fixed bug: ppp re-connect.
*         : 31.03.2014 2.00     Added setting variable "_t4_channel_num" for several link-layer channels.
*         :                     Change user defined function spec.
*         :                     lan_read(), lan_write(), lan_reset(),
*         :                     report_error(), and rcv_buff_release(),
*         :                     Change API define _process_tcpip().
*         :                     Change error code (E_XX) and NADR value.
*         :                     Deleted PPP Libraries temporary. (maybe return at next release)
*         :                     Fixed Bug.
*         : 07.08.2014 2.01     Clean up source code.
*         : 29.01.2015 2.02     Changed RX IDE from High-performance Embedded Workshop to CS+/e2 studio.
*         :                     Changed RX compiler option from (-rx600,-rx200) to (-rxv2,-rxv1).
*         : 01.07.2015 2.03     Fixed Bug: Fixed behavior of crossing-FIN sequence.
*         :                     Fixed Bug: Fixed behavior of receiving Windowsize=0 ACK when passive close.
*         : 25.08.2015 2.04     Added SC32 support. (Unofficial release)
*         : 01.12.2015 2.05     Added IGMP support.
*         : 30.11.2016 2.06     Added DHCP support.
*         : 12.12.2017 2.07     Fixed bug: Fixed behavior of UDP's cancele-callback sequence.
*         :                     Fixed bug: Fixed behavior of ICMP operation sequence.
*         : 10.12.2018 2.08     Fixed bug: Memory access violation due to the number of channels.
*         :                     Fixed bug: Transaction ID generation.
*         :                     Fixed bug: UDP callback routine.
*         :                     Fixed bug: DHCP sequence when connecting a repeatoer hub.
*         : 20.06.2019 2.09     Added TCP Keep-Alive function.
*         :                     Added support GCC RX compiler and IAR RX compiler .
*         :                     Changed specification T_TCP_CCEP, T_UDP_CCEP.
*         :                     Fixed bug: Subnetmask filtering.
*         :                     Fixed bug: API returns E_QOVR in Callback routine.
*         :                     Fixed bug: the callback routine is called when
*         :                     IP packet is received before IP address is bound.
*         : 29.01.2021 2.10    Added initial sequence number (ISN) generated by random number generator 
*                              for more security.
***********************************************************************************************************************/

#ifndef _R_T4_ITCPIP_H
#define _R_T4_ITCPIP_H

/*
 * ITRON data type definition
 */
#if defined(__MR30_H) || defined(__MR308_H) || defined(__MR32R) || defined(__HIOS_ITRON_H) || defined(__ITRON_H)
#define __ITRON_DATA_TYPE
#endif

#if !defined(__ITRON_DATA_TYPE)
#define __ITRON_DATA_TYPE
#if defined(R8C) || defined(M16C) || defined(M16C80) || defined(M32C80) ||\
    defined(__300HA__) || defined(__2600A__) ||\
    defined(_SH2) || defined(_SH2A) || defined(_SH2AFPU) || defined(_SH4) || defined(_SH4A) ||\
    defined(__RX) || defined(__v850) || defined(__GNUC__) || defined(__ICCRX__)

#if defined(__RX) || defined(__GNUC__) || defined(__ICCRX__)
#include <stdint.h>
#else
#include "r_stdint.h"
#endif

typedef int8_t          B;
typedef int16_t         H;
typedef int32_t         W;
typedef uint8_t         UB;
typedef uint16_t        UH;
typedef uint32_t        UW;
typedef int8_t          VB;
typedef int16_t         VH;
typedef int32_t         VW;
typedef void  *         VP;
typedef void    (*FP)(void);

typedef W               INT;
typedef UW              UINT;
typedef H               ID;
typedef H               PRI;
typedef W               TMO;
typedef H               HNO;
typedef W               ER;
typedef UH              ATR;
#endif
#endif

typedef INT             FN;

#include "r_mw_version.h"

/*
 * ITRON TCP/IP API Specifications header file
 */
/*********************************/
/*** Data structure definition ***/
/*********************************/
/*** IP address/Port No. information ***/
typedef struct t_ipv4ep
{
    UW       ipaddr;    /* IP address */
    UH       portno;    /* Port number */
} T_IPV4EP;

/***  TCP reception point  ***/
typedef struct t_tcp_crep
{
    ATR      repatr;    /* TCP reception point attribute    */
    T_IPV4EP myaddr;    /* Local IP address and port number */
} T_TCP_CREP;

/***  TCP communication end point  ***/
typedef struct t_tcp_ccep
{
    ATR      cepatr;     /* TCP communication end point attribute  */
    VP       sbuf;       /* Top address of transmit window buffer  */
    INT      sbufsz;     /* Size of transmit window buffer         */
    VP       rbuf;       /* Top address of receive window buffer   */
    INT      rbufsz;     /* Size of receive window buffer          */
    ER(*callback)(ID cepid, FN fncd , VP p_parblk);   /* Callback routine */
    UW lan_port_number;  /* LAN port number */
    UW keepalive_enable; /* Keep-alive function */
} T_TCP_CCEP;

/***  UDP communication end point  ***/
typedef struct t_udp_ccep
{
    ATR      cepatr;     /* UDP communication end point attribute  */
    T_IPV4EP myaddr;     /* Local IP address and port number       */
    ER(*callback)(ID cepid, FN fncd , VP p_parblk); /* Callback routine */
    UW lan_port_number;  /* LAN port number */
} T_UDP_CCEP;

/***  IP address settings  ***/
typedef struct
{
    UB ipaddr[4];       /* Local IP address        */
    UB maskaddr[4];     /* Subnet mask             */
    UB gwaddr[4];       /* Gateway address X       */
#if defined(__M32R__)
    UW dummy;           /* for alignment           */
#endif
} TCPUDP_ENV;

/*** statistics of T4 ***/
typedef struct T4_STATISTICS
{
    /* t4 status */
    UW t4_rec_cnt;
    UW t4_rec_byte;
    UW t4_snd_cnt;
    UW t4_snd_byte;

    /* report error */
    UW re_len_cnt;
    UW re_network_layer_cnt;
    UW re_transport_layer_cnt;
    UW re_arp_header1_cnt;
    UW re_arp_header2_cnt;
    UW re_ip_header1_cnt;
    UW re_ip_header2_cnt;
    UW re_ip_header3_cnt;
    UW re_ip_header4_cnt;
    UW re_ip_header5_cnt;
    UW re_ip_header6_cnt;
    UW re_ip_header7_cnt;
    UW re_ip_header8_cnt;
    UW re_ip_header9_cnt;
    UW re_tcp_header1_cnt;
    UW re_tcp_header2_cnt;
    UW re_udp_header1_cnt;
    UW re_udp_header2_cnt;
    UW re_udp_header3_cnt;
    UW re_icmp_header1_cnt;
    UW re_igmp_header1_cnt;    /* v205 IGMP ext. */
    UW re_igmp_header2_cnt;    /* v205 IGMP ext. */
    UW re_dhcp_header1_cnt;    /* v206 DHCP ext. */
    UW re_dhcp_header2_cnt;    /* v206 DHCP ext. */
} T4_STATISTICS;

typedef struct _dhcp
{
    uint8_t ipaddr[4];
    uint8_t maskaddr[4];
    uint8_t gwaddr[4];
    uint8_t dnsaddr[4];
    uint8_t dnsaddr2[4];
    char    domain[253 + 1];
    uint8_t macaddr[6];
} DHCP;

typedef ER(*callback_from_system_t)(UB channel, UW eventid, VP param);

/****************************/
/***  API Function Codes  ***/
/****************************/
#define TFN_TCP_CRE_REP -0x0201  /* Create TCP reception point               */
#define TFN_TCP_DEL_REP -0x0202  /* Delete TCP reception point               */
#define TFN_TCP_CRE_CEP -0x0203  /* Create TCP communication end point       */
#define TFN_TCP_DEL_CEP -0x0204  /* Delete TCP communication end point       */
#define TFN_TCP_ACP_CEP -0x0205  /* Wait for TCP connection request          */
#define TFN_TCP_CON_CEP -0x0206  /* TCP connection request(active open)      */
#define TFN_TCP_SHT_CEP -0x0207  /* TCP data transmission end                */
#define TFN_TCP_CLS_CEP -0x0208  /* TCP communication end point close        */
#define TFN_TCP_SND_DAT -0x0209  /* Transmission of TCP data                 */
#define TFN_TCP_RCV_DAT -0x020A  /* Reception of TCP data                    */
#define TFN_TCP_GET_BUF -0x020B  /* TCP Transmission buffer retrieval        */
#define TFN_TCP_SND_BUF -0x020C  /* Transmission of data in TCP buffer       */
#define TFN_TCP_RCV_BUF -0x020D  /* Get TCP reception data in the buffer     */
#define TFN_TCP_REL_BUF -0x020E  /* TCP Reception buffer release             */
#define TFN_TCP_SND_OOB -0x020F  /* Transmission of urgent TCP data          */
#define TFN_TCP_RCV_OOB -0x0210  /* Reception of urgent TCP data             */
#define TFN_TCP_CAN_CEP -0x0211  /* Cancel pending TCP operation             */
#define TFN_TCP_SET_OPT -0x0212  /* Set TCP communication end point option   */
#define TFN_TCP_GET_OPT -0x0213  /* Read TCP communication end point option  */
#define TFN_TCP_ALL     0x0000
#define TFN_UDP_CRE_CEP -0x0221  /* Create UDP communication end point       */
#define TFN_UDP_DEL_CEP -0x0222  /* Delete UDP communication end point       */
#define TFN_UDP_SND_DAT -0x0223  /* Transmission of UDP data                 */
#define TFN_UDP_RCV_DAT -0x0224  /* Reception of UDP data                    */
#define TFN_UDP_CAN_CEP -0x0225  /* Cancel pending UDP operation             */
#define TFN_UDP_SET_OPT -0x0226  /* Set UDP communication end point option   */
#define TFN_UDP_GET_OPT -0x0227  /* Read UDP communication end point option  */
#define TFN_UDP_ALL     0x0000

/************************/
/***  API Event code  ***/
/************************/
#define TEV_TCP_RCV_OOB  0x0201 /* TCP urgent data received      */
#define TEV_UDP_RCV_DAT  0x0221 /* UDP data received             */

/***************************
 *      Error Code         *
 ***************************/
#ifndef E_OK
#define E_OK                  0  /* Normal completion               */
#endif
#ifndef E_SYS
#define E_SYS                -5  /* System error                    */
#endif
#ifndef E_NOSPT
#define E_NOSPT              -9  /* Function not supported          */
#endif
#ifndef E_RSATR
#define E_RSATR             -11  /* Reserved attribute              */
#endif
#ifndef E_PAR
#define E_PAR               -17  /* Parameter error                 */
#endif
#ifndef E_ID
#define E_ID                -18  /* Invalid ID number               */
#endif
#ifndef E_MACV
#define E_MACV              -26  /* Memory access violation         */
#endif
#ifndef E_NOMEM
#define E_NOMEM             -33  /* Insufficient memory             */
#endif
#ifndef E_OBJ
#define E_OBJ               -41  /* Object state error              */
#endif
#ifndef E_NOEXS
#define E_NOEXS             -42  /* Object does not exist           */
#endif
#ifndef E_QOVR
#define E_QOVR              -43  /* Queuing overflow                */
#endif
#ifndef E_RLWAI
#define E_RLWAI             -49  /* Forced release from waiting     */
#endif
#ifndef E_TMOUT
#define E_TMOUT             -50  /* Timeout                         */
#endif
#ifndef E_DLT
#define E_DLT               -51  /* Waiting object deleted          */
#endif
#ifndef E_WBLK
#define E_WBLK              -57  /* Non-blocking call accept        */
#endif
#ifndef E_CLS
#define E_CLS               -52  /* Connection failure              */
#endif
#ifndef E_BOVR
#define E_BOVR              -58  /* Buffer overflow                 */
#endif

/***************/
/***  Other  ***/
/***************/
#ifndef TMO_POL
#define TMO_POL              0  /* Polling                                       */
#endif
#ifndef TMO_FEVR
#define TMO_FEVR            -1  /* Waiting forever                               */
#endif
#ifndef TMO_NBLK
#define TMO_NBLK            -2  /* Non-blocking call (Set timeout value)         */
#endif
#ifndef NADR
#define NADR                 0  /* IP address, port number specification omitted */
#endif
#ifndef IPV4_ADDRANY
#define IPV4_ADDRANY         0  /* IP address specification omitted              */
#endif
#ifndef TCP_PORTANY
#define TCP_PORTANY          0  /* TCP port number specification omitted         */
#endif
#ifndef UDP_PORTANY
#define UDP_PORTANY          0  /* UDP port number specification omitted         */
#endif

/******************************************
 *      Error code (report error)         *
 ******************************************/

#define RE_LEN              -1
#define RE_NETWORK_LAYER    -2
#define RE_TRANSPORT_LAYER  -3
#define RE_ARP_HEADER1      -21
#define RE_ARP_HEADER2      -22
#define RE_IP_HEADER1       -41
#define RE_IP_HEADER2       -42
#define RE_IP_HEADER3       -43
#define RE_IP_HEADER4       -44
#define RE_IP_HEADER5       -45
#define RE_IP_HEADER6       -46
#define RE_IP_HEADER7       -47
#define RE_IP_HEADER8       -48
#define RE_IP_HEADER9       -49
#define RE_TCP_HEADER1      -61
#define RE_TCP_HEADER2      -62
#define RE_UDP_HEADER1      -81
#define RE_UDP_HEADER2      -82
#define RE_UDP_HEADER3      -83
#define RE_ICMP_HEADER1     -101

#define RE_IGMP_HEADER1     -121 /* v205 IGMP ext. this error is IGMP header checksum error*/
#define RE_IGMP_HEADER2     -122 /* v205 IGMP ext. this error is IGMP packet Untreated*/

#define RE_DHCP_ILLEGAL     (-131)
#define RE_DHCP_SND_TIMEOUT (-132)

/******************************************
 *      Error code (IGMP)                 *
 ******************************************/

#define E_IGMP_MULTICAST_OUT_OF_RANGE -1
#define E_IGMP_MULTICAST_DOUBLE_ENTRY -2
#define E_IGMP_MULTICAST_NOT_ENTRY    -3
#define E_IGMP_MULTICAST_MAX_ENTRY    -4
#define E_IGMP_SYSTEM_ERROR           -5

#define IP_ALEN    4
typedef UB IPaddr[IP_ALEN]; /*  IP address */


/*******************************
 *      callback eventID       *
 *******************************/
/* L2:ether layer */
#define ETHER_EV_LINK_OFF           (0u)
#define ETHER_EV_LINK_ON            (1u)
#define ETHER_EV_COLLISION_IP       (2u)
/* L7:dhcp */
#define DHCP_EV_LEASE_IP            (20u)
#define DHCP_EV_LEASE_OVER          (21u)
#define DHCP_EV_INIT                (22u)
#define DHCP_EV_INIT_REBOOT         (23u)
#define DHCP_EV_APIPA               (24u)
#define DHCP_EV_NAK                 (25u)
#define DHCP_EV_FATAL_ERROR         (26u)
#define DHCP_EV_PLEASE_RESET        (27u)

/*******************************/
/***  Prototype Declaration  ***/
/*******************************/
#if defined(__GNUC__)
#if defined(__cplusplus)
extern "C"
{
#endif
#endif
ER udp_snd_dat(ID cepid, T_IPV4EP *p_dstaddr, VP data, INT len, TMO tmout);
ER udp_rcv_dat(ID cepid, T_IPV4EP *p_dstaddr, VP data, INT len, TMO tmout);
ER udp_can_cep(ID cepid, FN fncd);

ER tcp_acp_cep(ID cepid, ID repid, T_IPV4EP *p_dstadr, TMO tmout);
ER tcp_con_cep(ID cepid, T_IPV4EP *p_myadr,  T_IPV4EP *p_dstadr, TMO tmout);
ER tcp_sht_cep(ID cepid);
ER tcp_cls_cep(ID cepid, TMO tmout);
ER tcp_snd_dat(ID cepid, VP data,  INT dlen, TMO tmout);
ER tcp_rcv_dat(ID cepid, VP data, INT dlen, TMO tmout);
ER tcp_can_cep(ID cepid, FN fncd);

ER tcpudp_open(UW *workp);    /* Open TCP/IP library (initialization)                         */
ER tcpudp_close(void);        /* Close TCP/IP library (stop)                                  */
ER tcpudp_reset(UB channel);
W  tcpudp_get_ramsize(void);  /* Calculation of size of work area                             */
void _process_tcpip(void);    /* TCP/IP process function called from ether INT and timer INT. */
#if defined(__GNUC__)
#if defined(__cplusplus)
}
#endif
#endif

/* PPP-related APIs */
ER ppp_open(void);            /* Open PPP driver         */
ER ppp_close(void);           /* Close PPP driver        */
UH ppp_status(void);          /* PPP status              */


/**************************/
/***  Driver Interface  ***/
/**************************/
/*++++++++++++++++ PPP/Ether common items +++++++++++++++++*/
H    rcv_buff_release(UB lan_port_no);
void tcpudp_act_cyc(UB cycact);           /* Control TCP cyclic processing start/stop     */
void tcp_api_slp(ID cepid);               /* Wait for completion of TCP API               */
void tcp_api_wup(ID cepid);               /* Cancel the wait state of TCP API completion  */
void udp_api_slp(ID cepid);               /* Wait for completion of UDP API               */
void udp_api_wup(ID cepid);               /* Cancel the wait state of UDP API completion  */
void ppp_api_slp(void);                   /* Wait for completion of PPP API               */
void ppp_api_wup(void);                   /* Cancel the wait state of PPP API completion  */
UH   tcpudp_get_time(void);               /* Get time information                         */
void report_error(UB lan_port_no, H err_code, UB *err_data);      /* Report error function  */
void ena_int(void);                       /* temporarily enable interrupt function        */
void dis_int(void);                       /* temporarily disable interrupt function       */
void get_random_number(UB *data, UW len); /* Get random number for CHAP auth              */
void get_hash_value(UB lan_port_no, UB *message, UW message_len, UB **hash, UW *hash_len);  /* Get hash value for ISN */
void register_callback_linklayer(callback_from_system_t call_fp);
H lan_check_link(UB lan_port_no);

/* user publication function */
UW igmp_join_group(UW* mCastAdr, UW RJ45port);
UW igmp_leave_group(UW* mCastAdr, UW RJ45port);

/* T4 version information structure*/
extern const mw_version_t R_t4_version;

/* common variable in T4 */
extern TCPUDP_ENV tcpudp_env[];
extern const H __udpcepn;
extern const UB _t4_channel_num;
extern const callback_from_system_t g_fp_user;
extern const UH _t4_dhcp_ip_reply_arp_delay;

/*++++++++++++++++ PPP +++++++++++++++++*/
/* PPP authentication system (Set ppp_auth) */
#define AUTH_NON                1 /* No authentication          */
#define AUTH_PAP                2 /* PAP                        */
#define AUTH_CHAP_MD5           4 /* CHAP_MD5                   */

/* Serial data baud rate (sio_open() argument) */
#define BR96                0
#define BR192               1
#define BR288               2
#define BR384               3
#define BR576               4
#define BR1152              5

/* Serial transfer mode (ppp_sio_status) */
#define PPP_SIO_STAT_MODEM  1  /* Modem command transmission and reception mode */
#define PPP_SIO_STAT_PPP    2  /* PPP frame transmission and reception mode  */

/* ppp_status() return value */
#define PS_DEAD             0x0001 /* Initial state                   */
#define PS_ESTABLISH        0x0002 /* LCP phase                       */
#define PS_AUTHENTICATE     0x0004 /* Authentication phase (PAP)      */
#define PS_NETWORK          0x0008 /* NCP phase (IPCP)                */
#define PS_NETOPEN          0x0010 /* Established Network             */
#define PS_TERMINATE        0x0020 /* Disconnecting Link              */
#define PS_RESERVED         0xffc0 /* Reservation                     */

/* ppp_drv_status() return value */
#define PDS_SND             0x0001 /* 1: sending, 0: transmit buffer empty / sending completed */
#define PDS_SND_FULL        0x0002 /* 1: transmit buffer full, 0: transmit buffer empty        */
#define PDS_RCV             0x0010 /* 1: received data present, 0: receive buffer empty        */
#define PDS_RCV_FULL        0x0020 /* 1: receive buffer full, 0: receive buffer empty          */
#define PDS_RESERVED        0xffcc /* Reserved             */

/* ppp_api_req() argument type */
#define PPP_API_SNDCOMMAND  3  /* Request to send AT command              */
#define PPP_API_RCVRZLT     4  /* Request to receive response code        */
#define PPP_API_WAIT        5  /* Request to wait for a specified time    */

/* PPP mode */
#define PPP_MODE_SERVER     0x0001 /* server mode           */
#define PPP_MODE_CLIENT     0x0002 /* client mode           */

/* Dial-up information */
typedef struct
{
    UB *at_commands;               /* Modem setting for data communication    */
    UB *peer_dial;                 /*  Server telephone number                */
} DUP_INFO;
/* PPP connection information */
typedef struct
{
    UB src_ipaddr[4];              /* Local IP address         */
    UB dst_ipaddr[4];              /* Remote IP address        */
} PPP_CLIENT_INFO;

/* PPP driver API (called by user application)  */
void sio_open(UB rate);
void sio_close(void);
ER  modem_active_open(void);
ER  modem_passive_open(void);
ER  modem_close(void);

/* PPP driver interface function (called by library) */
H  ppp_read(UB **ppp);
H  ppp_write(B *hdr, H hlen, B **pdata, H *pdlen, H num);
ER ppp_api_req(UH type, void *parblk, H tmout);
UH ppp_drv_status(void);            /* Get PPP driver state      */
H  modem_read(UB **rzlt);
H  modem_write(void *parblk);


/*++++++++++++++++ Ether related +++++++++++++++++*/
#if defined(__GNUC__)
#if defined(__cplusplus)
extern "C"
{
#endif
#endif
ER lan_open(void);                   /* Initialize LAN driver         */
ER lan_close(void);                  /* Deactivate the LAN driver     */
H lan_read(UB lan_port_no, B **buf);
H  lan_write(UB lan_port_no, B *header, H header_len, B *data , H data_len);  /* Send LAN data                 */
void lan_reset(UB lan_port_no);
#if defined(__GNUC__)
#if defined(__cplusplus)
}
#endif
#endif

#endif /* _R_T4_ITCPIP_H */


