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
* File Name    : udp.c
* Version      : 2.10
* Description  : Processing for TCP API
* Website      : https://www.renesas.com/mw/t4
***********************************************************************************************************************/
/**********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 01.04.2014 1.00    First Release
*         : 30.11.2016 1.01    add DHCP relation
*         : 12.12.2017 2.07    Fixed cancel-callback behavior.
*         : 10.12.2018 2.08    Fixed udp-callback behavior.
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

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/

#if defined(_UDP)
_UDP_CB  *_udp_cb;
extern far const T_UDP_CCEP udp_ccep[];
extern far const H __udpcepn;
extern far const UB _udp_enable_zerochecksum[];
extern UB *data_link_buf_ptr;
#endif

extern _TX_HDR _tx_hdr;
extern UB _t4_dhcp_enable;

#if defined(_UDP)
/***********************************************************************************************************************
* Function Name: udp_rcv_dat
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
ER udp_rcv_dat(ID cepid, T_IPVxEP *p_dstaddr, VP data, INT len, TMO tmout)
{
    ER  ercd;
    _UDP_CB *pcb;
    _UDP_API_REQ *p;

    ercd = _udp_check_cepid_arg(cepid);
    if (ercd != E_OK)
    {
        return E_PAR;
    }

    if(cepid > __udpcepn)
    {
        if(1 == _ch_info_head[cepid - 1 - __udpcepn].ip_terminated_flag)
        {
            return E_SYS;
        }
    }
    else
    {
        if (1 == _ch_info_head[(udp_ccep[cepid - 1].lan_port_number)].ip_terminated_flag)
        {
            return E_SYS;
        }
    }

    ercd = _udp_check_len_arg(len);
    if (ercd != E_OK)
    {
        return E_PAR;
    }
    pcb = &_udp_cb[cepid - 1];
    p   = &pcb->req;

    if (pcb->stat & _UDP_CB_STAT_CALLBACK)
    {
        if (tmout != TMO_POL && tmout != TMO_NBLK)
        {
            return E_PAR;
        }
    }
    else
    {
        if (tmout == TMO_POL)
        {
            return E_PAR;
        }
    }

    if (tmout == TMO_POL)
    {
        if (pcb->stat & _UDP_CB_STAT_RCV)
        {
            if (pcb->rcv.len > len)
            {
                ercd = E_BOVR;
            }
            else
            {
                len = pcb->rcv.len;
                ercd = len;
            }
            memcpy(p_dstaddr, &pcb->rcv.dstaddr, sizeof(T_IPVxEP));
            memcpy(data, pcb->rcv.data, len);

            pcb->stat &= (~_UDP_CB_STAT_RCV);
        }
        else
        {
            ercd = E_TMOUT;
        }
    }
    else
    {

        if ((p->type != _UDP_API_NON) ||
                ((pcb->stat & (_UDP_CB_STAT_CALLBACK | _UDP_CB_STAT_LOCK))
                 == (_UDP_CB_STAT_CALLBACK | _UDP_CB_STAT_LOCK)))
        {
            return (E_QOVR);
        }

        dis_int();

        if ((pcb->stat & _UDP_CB_STAT_CALLBACK) == 0)
        {
            pcb->stat |= _UDP_CB_STAT_LOCK;
        }

        p->cepid = cepid;
        p->len  = len;
        p->data  = (uchar *)data;
        p->cancel_flag = 0;
        p->tmout = tmout;
        p->ercd  = &pcb->req.rcv_ercd;
        p->p_dstaddr = p_dstaddr;
        p->type  = _UDP_API_RCV_DAT;
        p->stat  = _UDP_API_STAT_UNTREATED;

        if ((pcb->stat & _UDP_CB_STAT_CALLBACK) == 0)
        {
            pcb->stat &= (~_UDP_CB_STAT_LOCK);
        }

        ena_int();

        if (tmout == TMO_NBLK)
        {
            ercd = E_WBLK;
        }
        else
        {
            _udp_api_slp(pcb, cepid);
            ercd = pcb->req.rcv_ercd;
            if(cepid > __udpcepn)
            {
                if(1 == _ch_info_head[cepid - 1 - __udpcepn].ip_terminated_flag)
                {
                    ercd = E_SYS;
                }
            }
            else
            {
                if (1 == _ch_info_head[(udp_ccep[cepid - 1].lan_port_number)].ip_terminated_flag)
                {
                    ercd = E_SYS;
                }
            }
        }
        pcb->req.rcv_ercd = ercd;
    }
    return (ercd);
}


/***********************************************************************************************************************
* Function Name: udp_snd_dat
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
ER udp_snd_dat(ID cepid, T_IPVxEP *p_dstaddr, VP data, INT len, TMO tmout)
{
    ER  ercd;
    _UDP_CB *pcb;
    _UDP_API_REQ *p;

#if 1
    ercd = _udp_check_cepid_arg(cepid);
    if (ercd != E_OK)
    {
        return E_PAR;
    }

    if(cepid > __udpcepn)
    {
        if(1 == _ch_info_head[cepid - 1 - __udpcepn].ip_terminated_flag)
        {
            return E_SYS;
        }
    }
    else
    {
        if (1 == _ch_info_head[(udp_ccep[cepid - 1].lan_port_number)].ip_terminated_flag)
        {
            return E_SYS;
        }
    }

    ercd = _udp_check_len_arg(len);
    if (ercd != E_OK)
    {
        return E_PAR;
    }
#endif
    if (tmout == TMO_POL)
    {
        return E_PAR;
    }

    pcb = &_udp_cb[cepid - 1];
    p   = &pcb->req;

    if (pcb->stat & _UDP_CB_STAT_CALLBACK)
    {
        if (tmout != TMO_NBLK)
        {
            return E_PAR;
        }
    }

    if ((p->type != _UDP_API_NON) ||
            ((pcb->stat & (_UDP_CB_STAT_CALLBACK | _UDP_CB_STAT_LOCK))
             == (_UDP_CB_STAT_CALLBACK | _UDP_CB_STAT_LOCK)))
    {
        return (E_QOVR);
    }

    dis_int();

    if ((pcb->stat & _UDP_CB_STAT_CALLBACK) == 0)
    {
        pcb->stat |= _UDP_CB_STAT_LOCK;
    }

    p->cepid = cepid;
    p->len  = len;
    p->data  = (uchar *)data;
    p->cancel_flag = 0;
    p->tmout = tmout;
    p->ercd  = &pcb->req.snd_ercd;
    p->p_dstaddr = p_dstaddr;
    p->type  = _UDP_API_SND_DAT;
    p->stat  = _UDP_API_STAT_UNTREATED;

    if ((pcb->stat & _UDP_CB_STAT_CALLBACK) == 0)
    {
        pcb->stat &= (~_UDP_CB_STAT_LOCK);
    }

    ena_int();

    if (tmout == TMO_NBLK)
    {
        ercd = E_WBLK;
    }
    else
    {
        _udp_api_slp(pcb, cepid);
        ercd = pcb->req.snd_ercd;
        if(cepid > __udpcepn)
        {
            if(1 == _ch_info_head[cepid - 1 - __udpcepn].ip_terminated_flag)
            {
                ercd = E_SYS;
            }
        }
        else
        {
            if (1 == _ch_info_head[(udp_ccep[cepid - 1].lan_port_number)].ip_terminated_flag)
            {
                ercd = E_SYS;
            }
        }
    }
    pcb->req.snd_ercd = ercd;

    return (ercd);
}

/***********************************************************************************************************************
* Function Name: udp_can_cep
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
ER udp_can_cep(ID cepid, FN fncd)
{
    ER  ercd;
    _UDP_CB *pcb;
    _UDP_API_REQ *p;
    ID cepid_tmp = cepid;
    FN fncd_tmp = fncd;

    ercd = _udp_check_cepid_arg(cepid);
    if (ercd != E_OK)
    {
        return E_PAR;
    }

    if(cepid > __udpcepn)
    {
        if(1 == _ch_info_head[cepid - 1 - __udpcepn].ip_terminated_flag)
        {
            return E_SYS;
        }
    }
    else
    {
        if (1 == _ch_info_head[(udp_ccep[cepid - 1].lan_port_number)].ip_terminated_flag)
        {
            return E_SYS;
        }
    }

    pcb = &_udp_cb[cepid - 1];
    p   = &pcb->req;

    if (pcb->stat & _UDP_CB_STAT_CALLBACK)
    {
        return E_NOSPT;
    }

    if ((pcb->stat & _UDP_CB_STAT_CALLBACK) == 0)
    {
        pcb->stat |= _UDP_CB_STAT_LOCK;
    }


    ercd = E_OBJ;

    dis_int();

    if (((cepid_tmp == p->cepid) && (fncd_tmp == _udp_api_type_to_fn(p->type))) || \
            ((cepid_tmp == p->cepid) && (fncd_tmp == TFN_UDP_ALL) && (_udp_api_type_to_fn(p->type) == TFN_UDP_SND_DAT))\
            || ((cepid_tmp == p->cepid) && (fncd_tmp == TFN_UDP_ALL) && (_udp_api_type_to_fn(p->type) == \
                    TFN_UDP_RCV_DAT)))
    {
        ercd = E_OK;

        p->cancel_flag = 1;
    }


    if ((pcb->stat & _UDP_CB_STAT_CALLBACK) == 0)
    {
        pcb->stat &= (~_UDP_CB_STAT_LOCK);
    }

    ena_int();

    return (ercd);
}

/***********************************************************************************************************************
* Function Name: _udp_rcv
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _udp_rcv(_IP_HDR *piph, _UDP_HDR *pudph)
{
    sint16  i;
    _TCPUDP_PHDR ph;
    uint16  dport;
    _UDP_CB  *pucb;
    uint16  cksum_tmp;
    sint16  endpoint;

    cksum_tmp = _tcpudp_cksum(piph, &ph);
    if (pudph->cksum != 0)
    {
        if (pudph->cksum == 0xffff)
        {
            if (cksum_tmp == 0xffff)
            {
                cksum_tmp = 0;
            }
        }
        if (cksum_tmp != 0)
        {
            report_error(_ch_info_tbl->_ch_num, RE_UDP_HEADER1, data_link_buf_ptr);
            goto __err__udp_rcv;
        }
    }
    else
    {
        _tcpudp_cksum(piph, &ph);
        if (_udp_enable_zerochecksum[_ch_info_tbl->_ch_num] != 0)
        {
            report_error(_ch_info_tbl->_ch_num, RE_UDP_HEADER2, data_link_buf_ptr);
            goto __err__udp_rcv;
        }
    }
    dport = net2hs(pudph->dst_port);

    /* WAIT_LOOP */
    for (i = 0; i < __udpcepn; i++)
    {
        if ((udp_ccep[i].myaddr.portno == dport) && (_ch_info_tbl->_ch_num == udp_ccep[i].lan_port_number))
        {
            if(0 == _ip_available_check(udp_ccep[i].lan_port_number))
            {
                pucb = &_udp_cb[i];
                _udp_rcv_sub(pucb, pudph, &ph);
                break;
            }
        }
    }
    if (1 == _t4_dhcp_enable)
    {
        /* WAIT_LOOP */
        for (i = __udpcepn; i < (__udpcepn + _t4_channel_num); i++)
        {
            if (((_ch_info_tbl->pt_udp_dhcp_ccep->myaddr.portno) == dport) &&
                    ((_ch_info_tbl->_ch_num) == (_ch_info_tbl->pt_udp_dhcp_ccep->lan_port_number)))
            {
                pucb = &_udp_cb[i];
                _udp_rcv_sub(pucb, pudph, &ph);
                break;
            }
        }
    }

    if (1 == _t4_dhcp_enable)
    {
        endpoint = __udpcepn + _t4_channel_num;
    }
    else
    {
        endpoint = __udpcepn;
    }
    if (i == endpoint)
    {
        report_error(_ch_info_tbl->_ch_num, RE_UDP_HEADER3, data_link_buf_ptr);
    }

__err__udp_rcv:
    rcv_buff_release(_ch_info_tbl->_ch_num);
    _ch_info_tbl->_p_rcv_buf.len = 0;
}

/***********************************************************************************************************************
* Function Name: _udp_rcv_sub
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
sint16 _udp_rcv_sub(_UDP_CB *pucb, _UDP_HDR *udph, _TCPUDP_PHDR *ph)
{
    T_UDP_CCEP far const *pcep;
    FN    fncd;
    uint16  sport;
    uint16  saddr[IP_ALEN / 2];
    uint16  len;
    uchar   *data;
    uint16  ip_dlen;
    ID      cepid;
    UH      count;
    _UDP_CB *tmp;
    UH      endpoint;

    fncd = 0;
    if ((pucb->req.type == _UDP_API_RCV_DAT)
            && (pucb->req.stat == _UDP_API_STAT_INCOMPLETE))
    {
        if (pucb->req.tmout == TMO_NBLK)
        {
            fncd = TFN_UDP_RCV_DAT;
        }
    }
    else
    {
        fncd = TEV_UDP_RCV_DAT;
    }

    len = net2hs(udph->len);
    ip_dlen = net2hs(ph->len);
    if ((len < sizeof(_UDP_HDR)) || (ip_dlen < sizeof(_UDP_HDR)))
    {
        return (-1);
    }
    if (ip_dlen < len)
    {
        return (-1);
    }
    len  -= sizeof(_UDP_HDR);
    pucb->req.rcv_ercd  = len;
    sport = net2hs(udph->src_port);
    data  = (uchar*)udph + sizeof(_UDP_HDR);
    net2hl_yn_xn(saddr, ph->src_addr);

    if (fncd != TEV_UDP_RCV_DAT)
    {
        if (len > pucb->req.len)
        {
            pucb->req.rcv_ercd = E_BOVR;
            len = pucb->req.len;
        }

        memcpy(pucb->req.data, data, len);
        _cpy_ipaddr(&pucb->req.p_dstaddr->ipaddr, saddr);
        pucb->req.p_dstaddr->portno = sport;

        pucb->req.type = _UDP_API_NON;
    }
    else
    {
        pucb->rcv.data = data;
        _cpy_ipaddr(&pucb->rcv.dstaddr.ipaddr, saddr);
        pucb->rcv.dstaddr.portno = sport;
        pucb->rcv.len = len;
        pucb->stat |= _UDP_CB_STAT_RCV;
    }

    cepid = (pucb   - _udp_cb) + 1;
    if (1 == _t4_dhcp_enable)
    {
        if (cepid > __udpcepn)
        {
            pcep = (_ch_info_head[cepid - 1 - __udpcepn].pt_udp_dhcp_ccep);
        }
        else
        {
            pcep = &udp_ccep[cepid - 1];
        }
    }
    else
    {
        pcep = &udp_ccep[cepid - 1];
    }

    if (fncd == 0)
    {
        *(pucb->req.ercd) = pucb->req.rcv_ercd;

        _udp_api_wup(pucb, cepid);
    }
    else
    {
        if (pcep->callback != NULL)
        {
            if (1 == _t4_dhcp_enable)
            {
                endpoint = __udpcepn + _t4_channel_num;
            }
            else
            {
                endpoint = __udpcepn;
            }
            /* WAIT_LOOP */
            if ((0 != (_ch_info_head[pcep->lan_port_number]._myipaddr[0])) || (__udpcepn < cepid))
            {
                for (count = 0; count < endpoint; count++)
                {
                    tmp = &_udp_cb[count];
                    tmp->stat |= _UDP_CB_STAT_CALLBACK;
                }
                if (0 == (_ch_info_head[udp_ccep[cepid - 1].lan_port_number].ip_terminated_flag))
                {
                    (*pcep->callback)(cepid, fncd, (VP)&pucb->req.rcv_ercd);
                }
            }
        }
        if (1 == _t4_dhcp_enable)
        {
            if (udph->src_port == hs2net(DHCP_SERVER_PORT))
            {
                dhcp_rcv_dat_int(pucb, udph, ph);
            }
            endpoint = __udpcepn + _t4_channel_num;
        }
        else
        {
            endpoint = __udpcepn;
        }
        /* WAIT_LOOP */
        for (count = 0; count < endpoint; count++)
        {
            tmp = &_udp_cb[count];
            tmp->stat &= (~(_UDP_CB_STAT_CALLBACK | _UDP_CB_STAT_RCV));
        }
    }
    return (0);
}

/***********************************************************************************************************************
* Function Name: _udp_snd
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _udp_snd(_TCPUDP_PHDR *ph)
{
    FN    fncd;
    T_UDP_CCEP far const *pcep;
    sint16      len;
    uint16      sum16;
    _UDP_CB     *pucb;
    _UDP_API_REQ *pureq;
    _UDP_HDR    *udph;
    sint16      ret;
    sint16      i;
    UH          count;
    _UDP_CB     *tmp;
    uint32_t    ul_magic_cookie;
    uint32_t    ul_your_ip_address;
    T_UDP_CCEP* pt_udp_dhcp_ccep;
    sint16      endpoint;

    if (_t4_dhcp_enable == 1)
    {
        endpoint = __udpcepn + _t4_channel_num;
    }
    else
    {
        endpoint = __udpcepn;
    }
    /* WAIT_LOOP */
    for (i = 0; i < endpoint; i++)
    {
        pucb  = &_udp_cb[i];
        pureq = &pucb->req;

        if (_t4_dhcp_enable == 1)
        {
            if (i >= __udpcepn)
            {
                _ch_info_tbl = &_ch_info_head[i - __udpcepn];
            }
            else
            {
                _ch_info_tbl = &_ch_info_head[udp_ccep[i].lan_port_number];
                if( 0 != _ip_available_check(_ch_info_tbl->_ch_num))
                {
                    continue;
                }
            }
            pt_udp_dhcp_ccep = _ch_info_tbl->pt_udp_dhcp_ccep;
        }
        else
        {
            _ch_info_tbl = &_ch_info_head[udp_ccep[i].lan_port_number];
        }

        if ((pucb->stat & _UDP_CB_STAT_SND) && (pureq->stat == _UDP_API_STAT_INCOMPLETE))
        {
            udph = (_UDP_HDR *)(_tx_hdr.ihdr.tip.thdr.udph);
            if (_t4_dhcp_enable == 1)
            {
                if (i >= __udpcepn)
                {
                    udph->src_port = hs2net(pt_udp_dhcp_ccep->myaddr.portno);
                }
                else
                {
                    udph->src_port = hs2net(udp_ccep[i].myaddr.portno);
                }
            }
            else
            {
                udph->src_port = hs2net(udp_ccep[i].myaddr.portno);
            }
            udph->dst_port = hs2net(pureq->p_dstaddr->portno);
            len    = pureq->len + sizeof(_UDP_HDR);
            udph->len  = hs2net(len);
            udph->cksum  = 0;

            ph->len      = udph->len;
            ph->reserve  = 0;
            hl2net_yn_xn(&ph->dst_addr, &pureq->p_dstaddr->ipaddr);
            if (_t4_dhcp_enable == 1)
            {
                ul_magic_cookie = net2hl(*((uint32_t*)(((dhcp_packet_t*)pureq->data)->uca312_options)));
                ul_your_ip_address = _ch_info_tbl->_myipaddr[0];
                if ((ul_your_ip_address == 0) && (ul_magic_cookie == DHCP_PKTFLD_MAGIC_COOKIE))
                {
                    memset(ph->src_addr, 0, IP_ALEN);
                }
                else
                {
                    _cpy_ipaddr(ph->src_addr, _ch_info_tbl->_myipaddr);
                }
            }
            else
            {
                _cpy_ipaddr(ph->src_addr, _ch_info_tbl->_myipaddr);
            }
            ph->proto = _IPH_UDP;

            sum16 = _cksum((uchar *)ph,   sizeof(_TCPUDP_PHDR), 0);
            sum16 = _cksum((uchar *)udph, sizeof(_UDP_HDR), ~hs2net(sum16));
            sum16 = _cksum((uchar *)pureq->data, pureq->len, ~hs2net(sum16));
            udph->cksum = hs2net(sum16);

            if (udph->cksum == 0)
            {
                udph->cksum = 0xffff;
            }

            _tx_hdr.hlen = sizeof(_UDP_HDR);
            _cpy_ipaddr(_tx_hdr.ihdr.tip.iph.ip_dst, ph->dst_addr);
            if (_t4_dhcp_enable == 1)
            {
                if ((ul_your_ip_address == 0) && (ul_magic_cookie == DHCP_PKTFLD_MAGIC_COOKIE))
                {
                    memset(_tx_hdr.ihdr.tip.iph.ip_src, 0, IP_ALEN);
                }
                else    /*normal*/
                {
                    _cpy_ipaddr(_tx_hdr.ihdr.tip.iph.ip_src, ph->src_addr);
                }
            }
            else
            {
                _cpy_ipaddr(_tx_hdr.ihdr.tip.iph.ip_src, ph->src_addr);
            }
            _tx_hdr.ihdr.tip.iph.ip_proto_num = _IPH_UDP;
            ret = _ip_snd(pureq->data, pureq->len);

            if (ret >= 0 || -2 == ret)
            {
                if (-2 == ret)
                {
                    pureq->snd_ercd = E_CLS;
                }
                else
                {
                    pureq->snd_ercd = pureq->len;
                }
                pucb->stat &= ~(_UDP_CB_STAT_SND);

                if (pureq->tmout == TMO_NBLK)
                {
                    pureq->stat = _UDP_API_STAT_COMPLETE;
                    pureq->type = _UDP_API_NON;
                    if (_t4_dhcp_enable == 1)
                    {
                        if (i >= __udpcepn)
                        {
                            pcep = (_ch_info_head[i - __udpcepn].pt_udp_dhcp_ccep);
                        }
                        else
                        {
                            pcep = &udp_ccep[i];
                        }
                    }
                    else
                    {
                        pcep = &udp_ccep[i];
                    }
                    if (pcep->callback != NULL)
                    {
                        if (_t4_dhcp_enable == 1)
                        {
                            endpoint = __udpcepn + _t4_channel_num;
                        }
                        else
                        {
                            endpoint = __udpcepn;
                        }
                        /* WAIT_LOOP */
                        for (count = 0; count < endpoint; count++)
                        {
                            tmp = &_udp_cb[count];
                            tmp->stat |= _UDP_CB_STAT_CALLBACK;
                        }
                        fncd = TFN_UDP_SND_DAT;
                        if (0 == (_ch_info_tbl->ip_terminated_flag))
                        {
                            (*pcep->callback)(i + 1, fncd, (VP)&pureq->snd_ercd);
                        }
                    }
                    if (_t4_dhcp_enable == 1)
                    {
                        endpoint = __udpcepn + _t4_channel_num;
                    }
                    else
                    {
                        endpoint = __udpcepn;
                    }
                    /* WAIT_LOOP */
                    for (count = 0; count < endpoint; count++)
                    {
                        tmp = &_udp_cb[count];
                        tmp->stat &= ~(_UDP_CB_STAT_CALLBACK);
                    }
                }
                else
                {
                    *(pureq->ercd) = pureq->snd_ercd;

                    _udp_api_wup(pucb, i + 1);
                }
            }
        }
    }
}

/***********************************************************************************************************************
* Function Name: _udp_api_slp
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _udp_api_slp(_UDP_CB *pcb, ID id)
{
    do
    {
        udp_api_slp(id);
    }
    while (pcb->req.stat != _UDP_API_STAT_COMPLETE);    /* WAIT_LOOP */

    pcb->req.type = _UDP_API_NON;
}

/***********************************************************************************************************************
* Function Name: _udp_api_wup
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _udp_api_wup(_UDP_CB *pcb, ID id)
{
    if (pcb->req.stat != _UDP_API_STAT_COMPLETE)
    {
        pcb->req.stat = _UDP_API_STAT_COMPLETE;
        udp_api_wup(id);
    }
}


/***********************************************************************************************************************
* Function Name: _proc_udp_api
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _proc_udp_api(void)
{
    _UDP_CB      *pucb;
    _UDP_API_REQ *pureq;
    sint16      i;
    ER          ercd;
    FN          fn;
    UH          count;
    _UDP_CB     *tmp;
    T_UDP_CCEP* pt_udp_dhcp_ccep = _ch_info_tbl[0].pt_udp_dhcp_ccep;
    sint16      endpoint;

    if (_t4_dhcp_enable == 1)
    {
        endpoint = __udpcepn + _t4_channel_num;
    }
    else
    {
        endpoint = __udpcepn;
    }
    /* WAIT_LOOP */
    for (i = 0; i < endpoint; i++)
    {
        pucb  = &_udp_cb[i];
        pureq = &pucb->req;
        if (pureq->type != _UDP_API_NON)
        {
            if (pureq->stat == _UDP_API_STAT_UNTREATED)
            {
                pureq->stat = _UDP_API_STAT_INCOMPLETE;
                if (pureq->type == _UDP_API_SND_DAT)
                {
                    pucb->stat |= _UDP_CB_STAT_SND;
                }
            }
            if (pureq->cancel_flag == 1)
            {
                fn = _udp_api_type_to_fn(pureq->type);
                memset(pucb, 0, sizeof(_UDP_CB));
                ercd = E_RLWAI;
                pureq->stat = _UDP_API_STAT_COMPLETE;
                /* WAIT_LOOP */
                for (count = 0; count < endpoint; count++)
                {
                    tmp = &_udp_cb[count];
                    tmp->stat |= _UDP_CB_STAT_CALLBACK;
                }
                if (i >= __udpcepn)
                {
                    ((pt_udp_dhcp_ccep + (i - __udpcepn))->callback)(i + 1 /* cepid */, fn, (VP)&ercd);
                }
                else
                {
                    if (0 == (_ch_info_head[udp_ccep[i].lan_port_number].ip_terminated_flag))
                    {
                        (udp_ccep[i].callback)(i + 1 /* cepid */, fn, (VP)&ercd);
                    }
                }
                /* WAIT_LOOP */
                for (count = 0; count < endpoint; count++)
                {
                    tmp = &_udp_cb[count];
                    tmp->stat &= (~(_UDP_CB_STAT_CALLBACK));
                }
            }
        }
    }
}

/***********************************************************************************************************************
* Function Name: _udp_init
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _udp_init(UW **workpp)
{
    uint32_t    endpoint;

    _udp_cb = (_UDP_CB *)(*workpp);
    if (_t4_dhcp_enable == 1)
    {
        endpoint = __udpcepn + _t4_channel_num;
    }
    else
    {
        endpoint = __udpcepn;
    }
    *workpp = (UW *)((uchar *)(*workpp) + (sizeof(_UDP_CB) * (endpoint)));
}


/***********************************************************************************************************************
* Function Name: _udp_api_tmout
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
void _udp_api_tmout(void)
{
    _UDP_API_REQ *pureq;
    sint16      i;
    sint16      endpoint;

    if (_t4_dhcp_enable == 1)
    {
        endpoint = __udpcepn + _t4_channel_num;
    }
    else
    {
        endpoint = __udpcepn;
    }
    /* WAIT_LOOP */
    for (i = 0; i < endpoint; i++)
    {
        pureq = &_udp_cb[i].req;
        if (pureq->type != _UDP_API_NON)
        {
            if (pureq->stat == _UDP_API_STAT_INCOMPLETE)
            {
                if (pureq->tmout == 0)
                {
                    *(pureq->ercd) = E_TMOUT;

                    _udp_api_wup(&_udp_cb[i], i + 1);
                }
                if (pureq->tmout > 0)
                {
                    pureq->tmout--;
                }
            }
        }
    }
}
#endif  /*#if defined(_UDP)*/

/***********************************************************************************************************************
* Function Name: _tcpudp_cksum
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
uint16 _tcpudp_cksum(_IP_HDR *piph, _TCPUDP_PHDR *ph)
{
    uint16   sum16;
    uint16   len;

    _cpy_ipaddr(ph->src_addr, piph->ip_src);
    _cpy_ipaddr(ph->dst_addr, piph->ip_dst);
    ph->reserve = 0;
    ph->proto = piph->ip_proto_num;
    len    = piph->ip_total_len;

    len = net2hs(len);
    len -= _IP_HLEN_MIN;
    ph->len = hs2net(len);

    sum16 = _cksum((uchar *)ph, sizeof(_TCPUDP_PHDR), 0);
    sum16 = _cksum((uchar *)piph + sizeof(_IP_HDR), len, ~hs2net(sum16));

    return (sum16);
}

/***********************************************************************************************************************
* Function Name: _udp_api_type_to_fn
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
FN  _udp_api_type_to_fn(uint16 api_type)
{
    FN fncd = TFN_TCP_ALL;

    switch (api_type)
    {
        case _UDP_API_SND_DAT:
            fncd = TFN_UDP_SND_DAT;
            break;
        case _UDP_API_RCV_DAT:
            fncd = TFN_UDP_RCV_DAT;
            break;
        default:
            break;
    }

    return fncd;
}

/***********************************************************************************************************************
* Function Name: _udp_check_cepid_arg
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
ER _udp_check_cepid_arg(ID cepid)
{
    ER err = E_OK;
    ID      endpoint;

    if (_t4_dhcp_enable == 1)
    {
        endpoint = __udpcepn + _t4_channel_num;
    }
    else
    {
        endpoint = __udpcepn;
    }
    if ((cepid <= 0) || (cepid > endpoint))
    {
        err = E_PAR;
    }
    return err;
}

/***********************************************************************************************************************
* Function Name: _udp_check_len_arg
* Description  :
* Arguments    :
* Return Value :
***********************************************************************************************************************/
ER _udp_check_len_arg(INT len)
{
    ER err = E_OK;
    if (len < 0)
    {
        err = E_PAR;
    }
    return err;
}


