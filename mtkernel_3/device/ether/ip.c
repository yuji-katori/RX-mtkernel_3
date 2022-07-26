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
* Copyright (C) 2014-2021 Renesas Electronics Corporation. All rights reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : ip.c
* Version      : 2.10
* Description  : Processing for IP protocol
* Website      : https://www.renesas.com/mw/t4
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 01.04.2014 1.00    First Release
*         : 30.11.2016 1.01    File Header maintenance
*         : 20.06.2019 2.09    Fixed bug: the callback routine is called when
*         :                    IP packet is received before IP address is bound.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes   <System Includes> , "Project Includes"
***********************************************************************************************************************/
#include "t4define.h"

#include <string.h>
#include "type.h"
#include "r_t4_itcpip.h"
#if defined(_ETHER)
#include "ether.h"
#elif defined(_PPP)
#include "ppp.h"
#endif
#include "ip.h"
#include "tcp.h"
#include "udp.h"

#if defined(_IGMP)
#include "igmp.h"
#endif
#include "dhcp.h"

/***********************************************************************************************************************
Macro definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Typedef definitions
***********************************************************************************************************************/

/***********************************************************************************************************************
Exported global variables (to be accessed by other files)
***********************************************************************************************************************/
UB *data_link_buf_ptr;      /* Buffer pointer to Datalink layer */
_CH_INFO *_ch_info_tbl;
_CH_INFO *_ch_info_head;

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
static sint16 _ip_chk_srcip(uchar *src_ipaddr);

/***********************************************************************************************************************
Exported global variables (read from other files)
***********************************************************************************************************************/

extern _TX_HDR  _tx_hdr;    /* Transmit header area */
extern UB _t4_dhcp_enable;

#if defined(_MULTI)
extern TCPUDP_ENV tcpudp_env[];
extern far UB const __multi_TTL[];
#endif


/***********************************************************************************************************************
* Function Name: _ip_rcv_hdr
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
sint16 _ip_rcv_hdr(void)
{
    _IP_HDR  *pip;
    uint16  total_len;

    /* Head of IP packet   */
    pip  = (_IP_HDR *)_ch_info_tbl->_p_rcv_buf.pip;

#if defined(_MULTI)
    /* Enabled: Broadcast or Multicast */
    if (E_OK != _ip_check_ipadd_proto(pip))
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER1, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }
#else
    /* Disabled: Broadcast or Multicast */
    /* Check destination IP address: Only accept my IP address */
    if (_cmp_ipaddr(pip->ip_dst, _ch_info_tbl->_myipaddr) != 0)
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER1, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }
#endif

    /* Check source IP address (discard multicast and broadcast address) */
    if (pip->ip_src[0] >= 0xE0)
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER2, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }
    /* Check source IP address (discard loopback address 172.0.0.1) */
    if ((pip->ip_src[0] == 127) && (pip->ip_src[1] == 0)
            && (pip->ip_src[2] == 0)   && (pip->ip_src[3] == 1))
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER3, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }

    /* Check IP version */
    if ((pip->ip_ver_len >> 4) != _IPH_VERSION4)
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER4, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }

    /* exist IP options? (IP option is not supported) */
    /*   - If IP header length is not minimum size, this means this packet includes IP options */
    if (((pip->ip_ver_len & 0xf) << 2) != _IP_HLEN_MIN)
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER5, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }

    /* Check checksum */
    if (_cksum((uchar *)pip, _IP_HLEN_MIN, 0) != 0)
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER6, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }

    /* Check IP datagram length is wrong.
     *  - IP datagram length in IP header is bigger than driver notified (include padding)
     *  - IP datagram length in IP header is smaller than IP header minimum size
     */
    total_len = net2hs(pip->ip_total_len);
    if ((total_len > _ch_info_tbl->_p_rcv_buf.len) || (total_len < _IP_HLEN_MIN))
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER7, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }

    /* Check invalid source IP address */
    if (_ip_chk_srcip(pip->ip_src) != 0)
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER8, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }

    /* Not supported fragment datagram */
    if ((pip->ip_fragoff & hs2net(_IPH_FRAGOFF | _IPH_MF)) != 0)
    {
        report_error(_ch_info_tbl->_ch_num, RE_IP_HEADER9, data_link_buf_ptr);
        goto _err_ip_rcv_hdr;
    }
    return 0;

_err_ip_rcv_hdr:
    return -1;
}

/***********************************************************************************************************************
* Function Name: _ip_snd
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
sint16 _ip_snd(uchar *data, uint16 dlen)
{
    sint16 ercd;
    register _IP_HDR *pip = (_IP_HDR *) & _tx_hdr.ihdr.tip.iph;
    register uint16 sum;

    /* generate: IP header */
    pip->ip_chksum  = 0;
    pip->ip_tos   = 0;
    pip->ip_fragoff  = 0;
    pip->ip_ver_len  = (_IPH_VERSION4 << 4) | (_IP_HLEN_MIN >> 2);
#if defined(_MULTI)
    if ((pip->ip_dst[0] & 0xf0) == 0xe0)   /* multicast address */
    {
        pip->ip_ttl  = __multi_TTL[_ch_info_tbl->_ch_num];
    } /*2669*/
    else
#endif
    {
        pip->ip_ttl   = _IPH_TTL;
    } /*2669*/
    pip->ip_total_len = _IP_HLEN_MIN + _tx_hdr.hlen + dlen;
    pip->ip_total_len = hs2net(pip->ip_total_len);
    pip->ip_id   = hs2net(_ch_info_tbl->_ip_id);
    _ch_info_tbl->_ip_id++;

    sum  = _cksum((uchar *)pip, _IP_HLEN_MIN, 0);
    pip->ip_chksum  = hs2net(sum);

    _tx_hdr.hlen += _IP_HLEN_MIN;

#if defined(_PPP)
    ercd = _ppp_snd_ip(data, dlen);
#elif defined(_ETHER)
    ercd = _ether_snd_ip(data, dlen);
#endif
    if (ercd == 0)
    {
        return E_IP_SENT;
    }
    else if (ercd == -2)
    {
        return -2;
    }
    else
    {
        return E_IP_PENDING;
    }
}

#if defined(_ICMP)
/***********************************************************************************************************************
* Function Name: _ip_snd_icmp
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _ip_snd_icmp(void)
{
    _ch_info_tbl->flag |= _CHINFOF_SND_ICMP;
    return;
}
#endif

/***********************************************************************************************************************
* Function Name: _ip_chk_srcip
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
static sint16 _ip_chk_srcip(uchar *src_ip)
{
    uint32 tmp1, tmp2, tmp3 ;

    /* Check if src IP is Broadcast or Multicast */
    if (src_ip[0] >= 0xE0)
    {
        return -1;
    }

#if defined(_ETHER)
    /* Check if src IP is Network Address or Network Broadcast */
    _cpy_ipaddr(&tmp1, src_ip);
    _cpy_ipaddr(&tmp2, _ch_info_tbl->_mymaskaddr);
    _cpy_ipaddr(&tmp3, _ch_info_tbl->_myipaddr);
    if ((tmp1 == (tmp2 & tmp3)) || (tmp1 == (~tmp2 | tmp3)))
    {
        return -1;
    }

    return 0;
#endif
}

#if defined(_MULTI)
/***********************************************************************************************************************
* Function Name: _ip_check_ipadd_proto
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
schar _ip_check_ipadd_proto(_IP_HDR *piph)
{
    uchar type;
    _UDP_PKT  *pudp;
    _UDP_HDR  *pudph;


    if (_cmp_ipaddr(piph->ip_dst, _ch_info_tbl->_myipaddr) == 0)
    {
        if ((piph->ip_proto_num == _IPH_TCP)
#if defined(_ICMP)
                || (piph->ip_proto_num == _IPH_ICMP)
#endif
#if defined(_UDP)
                || (piph->ip_proto_num == _IPH_UDP)
#endif
           )
        {
            return (0);
        }
    }
    else
    {
        type = _ip_check_broadcast(piph->ip_dst);
        if (type == _IP_TYPE_BROADCAST)
        {
            if (piph->ip_proto_num != _IPH_TCP)
            {
                return (0);
            } /*2669*/
        }
        else if (type == _IP_TYPE_DIRECTED_BROADCAST)
        {
            if (piph->ip_proto_num != _IPH_TCP)
            {
                return (0);
            }
        }

        type = _ip_check_multicast(piph->ip_dst);
        if (type == _IP_TYPE_MULTI_ALL_HOST)
        {
            if (piph->ip_proto_num != _IPH_TCP)
            {
#if defined(_IGMP)
                if(0 == _ip_available_check(_ch_info_tbl->_ch_num))
                {
                    int_send_igmp_when_router_query_receive(_ch_info_tbl->_ch_num, &(_ch_info_tbl->_p_rcv_buf));
                }
#endif
                return (0);
            }
        }
        else if (type == _IP_TYPE_MULTI_ANY)
        {
            if (piph->ip_proto_num != _IPH_TCP)
            {
#if defined(_IGMP)
                if(0 == _ip_available_check(_ch_info_tbl->_ch_num))
                {
                    int_send_igmp_when_membership_report_receive(_ch_info_tbl->_ch_num, &(_ch_info_tbl->_p_rcv_buf));
                }
#endif
                return (0);
            }
        }
        if (1 == _t4_dhcp_enable)
        {
            pudp  = (_UDP_PKT *)(((_IP_PKT*)piph)->data);
            pudph = (_UDP_HDR *)pudp;
            if (((_IPH_UDP == (piph->ip_proto_num)) && ((pudph->src_port) == hs2net(DHCP_SERVER_PORT)))
                    || ((pudph->dst_port) == hs2net(DHCP_CLIENT_PORT)))
            {
                return (0);
            }
        }
    }
    return (-1);
}

/***********************************************************************************************************************
* Function Name: _ip_check_multicast
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
uchar _ip_check_multicast(uchar *ipaddr)
{
    uint32 addr;

    net2hl_yn_xn(&addr, ipaddr);

    if (addr == 0xe0000000u)
    {
        return (_IP_TYPE_MULTI_RESERVED);
    } /*2669*/
    else if (addr == 0xe0000001u)
    {
        return (_IP_TYPE_MULTI_ALL_HOST);
    } /*2669*/
    else if ((ipaddr[0] & 0xf0) == 0xe0)
    {
        return (_IP_TYPE_MULTI_ANY);
    } /*2669*/
    else
    {
        return (_IP_TYPE_NON_MULTI);
    } /*2669*/
}

/***********************************************************************************************************************
* Function Name: _ip_check_broadcast
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
uchar _ip_check_broadcast(uchar *ipaddr)
{
    uint32 addr;
    uint32 myipaddr;
    uint32 subnet_mask;
    uint32 broad_cast_addr = 0xffffffffu;

    net2hl_yn_xn(&addr, ipaddr);
    net2hl_yn_xn(&subnet_mask, tcpudp_env[_ch_info_tbl->_ch_num].maskaddr);
    net2hl_yn_xn(&myipaddr, tcpudp_env[_ch_info_tbl->_ch_num].ipaddr);

    if (0xffffffffu == addr)
    {
        return (_IP_TYPE_BROADCAST);
    } /*2669*/
    else if ((addr & ~subnet_mask) == (broad_cast_addr & ~subnet_mask))
    {
        if ((addr & subnet_mask) == (myipaddr & subnet_mask))
        {
            return (_IP_TYPE_DIRECTED_BROADCAST);
        }
        else
        {
            return (_IP_TYPE_NON_BROAD);
        }
    }
    else
    {
        return (_IP_TYPE_NON_BROAD);
    }
}
#endif

/***********************************************************************************************************************
* Function Name: _ip_available_check
* Description  : Check if the IP address is available
* Arguments    : UB channel
*              :     lan port number
* Return Value :  0: available
*              : -1: not available
***********************************************************************************************************************/
int32_t _ip_available_check(uint8_t channel)
{
    int32_t ret;
    ret = dhcp_ip_available_check(channel);
    return ret;
}
