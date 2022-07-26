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
* Copyright (C) 2015-2021 Renesas Electronics Corporation, All Rights Reserved.
***********************************************************************************************************************/
/***********************************************************************************************************************
* File Name    : igmp.c
* Version      : 2.10
* Description  : igmp client function
* Website      : https://www.renesas.com/mw/t4
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 12.11.2015 1.00    First Release
*         : 30.11.2016 1.01    File Header maintenance
*         : 20.06.2019 2.09    Fixed bug: the callback routine is called when
*         :                    IP packet is received before IP address is bound.
***********************************************************************************************************************/

/***********************************************************************************************************************
Includes <System Includes> ,
***********************************************************************************************************************/
#include "t4define.h"

#include <stdio.h>
#include <string.h>
#include "type.h"
#include "r_t4_itcpip.h"

#include "igmp.h"

/*#define debug*/

#define C10SEC  1000                                                /* 10ms int 1000 count = 10x1000=10sec */

/***********************************************************************************************************************
Private global variables and functions
***********************************************************************************************************************/
static const uint8_t IGMP_BENDER_CODE[3] = {0x01, 0x00, 0x5E};             /* multicast packet mac head 3byte */


extern UH get_timer(void);

extern UB   _myethaddr[][6];
extern TCPUDP_ENV tcpudp_env[];
extern UB *data_link_buf_ptr;

static IGMP_GROUP_ARRAY IGMP_WK[MULTI_CAST_MAX_GROUP_COUNT];               /* multicast group memory */


/***********************************************************************************************************************
* Function Name: make_igmp_etherLayerHeader
* Description  : make_igmp_etherLayerHeader
* Arguments    : sendPacketPt
*              :  data pointer
*              : settingPt
*              :  work pointer
* Return Value : none
***********************************************************************************************************************/
static void make_igmp_etherLayerHeader(IGMP_PACKET_EX *sendPacketPt, IGMP_GROUP_ARRAY *settingPt)
{
    memcpy(sendPacketPt->etherLay.eh_src, _myethaddr[settingPt->RJ45port], EP_ALEN);
    sendPacketPt->etherLay.eh_dst[0] = IGMP_BENDER_CODE[0];         /* multicast address head 3byte (RFC compliance) */
    sendPacketPt->etherLay.eh_dst[1] = IGMP_BENDER_CODE[1];
    sendPacketPt->etherLay.eh_dst[2] = IGMP_BENDER_CODE[2];
    sendPacketPt->etherLay.eh_dst[3] = settingPt->mCastAdr[1] & 0x7F;           /* multicast address low 23bit entry */
    sendPacketPt->etherLay.eh_dst[4] = settingPt->mCastAdr[2];
    sendPacketPt->etherLay.eh_dst[5] = settingPt->mCastAdr[3];

    sendPacketPt->etherLay.eh_type = hs2net(EPT_IP);
}

/***********************************************************************************************************************
* Function Name: make_igmp_IPv4LayerHeader
* Description  : make_igmp_IPv4LayerHeader
* Arguments    : sendPacketPt
*              :  data pointer
*              : settingPt
*              :  work pointer
* Return Value : none
***********************************************************************************************************************/
static void make_igmp_IPv4LayerHeader(IGMP_PACKET_EX *sendPacketPt, IGMP_GROUP_ARRAY *settingPt)
{
    UB  allRouter[] = {224, 0, 0, 2};

    sendPacketPt->ipv4Lay.iphdr.ip_tos = 0x30;
    sendPacketPt->ipv4Lay.iphdr.ip_total_len = hs2net(sizeof(IGMP_PACKET));
    sendPacketPt->ipv4Lay.iphdr.ip_id = hs2net(0x1234);
    sendPacketPt->ipv4Lay.iphdr.ip_fragoff = 0;
    sendPacketPt->ipv4Lay.iphdr.ip_ttl = 1;
    sendPacketPt->ipv4Lay.iphdr.ip_proto_num = _IPH_IGMP;
    sendPacketPt->ipv4Lay.iphdr.ip_chksum  = 0;
    memcpy(sendPacketPt->ipv4Lay.iphdr.ip_src, tcpudp_env[settingPt->RJ45port].ipaddr, IP_ALEN);
    if (settingPt->leaveFlag == 1)                                  /* ip destination address IGMPv2 = 224.0.0.2 */
    {
        memcpy(sendPacketPt->ipv4Lay.iphdr.ip_dst, allRouter, IP_ALEN);
    }
    else                                                            /* Other than that all IGMP header */
    {
        memcpy(sendPacketPt->ipv4Lay.iphdr.ip_dst, settingPt->mCastAdr, IP_ALEN);
    }
    if (settingPt->igmpMode == ENM_IGMPV1_TYPE)                     /* IGMPv1 IP option not added */
    {
        sendPacketPt->ipv4Lay.iphdr.ip_ver_len = (_IPH_IP << 4) | (sizeof(_IP_HDR) >> 2);
        sendPacketPt->ipv4Lay.iphdr.ip_chksum = hs2net(_cksum((uchar *) & (sendPacketPt->ipv4Lay), sizeof(_IP_HDR), 0));
    }
    else if (settingPt->igmpMode == ENM_IGMPV2_TYPE)                /* IGMPv2 */
    {
        sendPacketPt->ipv4Lay.option = hl2net(0x94040000u);         /* IGMPv2 IP header option area added RFC2336 */
        sendPacketPt->ipv4Lay.iphdr.ip_ver_len = (_IPH_IP << 4) | (sizeof(_IP_HDR_EX) >> 2);
        sendPacketPt->ipv4Lay.iphdr.ip_chksum = hs2net(_cksum((uchar *) &
                                                (sendPacketPt->ipv4Lay), sizeof(_IP_HDR_EX), 0));
    }
    else
    {
        /* nothing to do */
    }

}


/***********************************************************************************************************************
* Function Name: igmp_multicast_address_check
* Description  : igmp_multicast_address_check
* Arguments    : mCastAdr
*              :  ip address work pointer
* Return Value : 0
*              :  out of range
*              : 1
*              :  success
***********************************************************************************************************************/
static int32_t igmp_multicast_address_check(UB* mCastAdr)
{
    int32_t res = 0;                                            /* err:out of range */
    UB  multiStart[] = {224, 0, 1, 0};
    UB  multiEnd[]   = {239, 255, 255, 255};

#ifdef debug
    unsigned char *prtPtr = (unsigned char*)mCastAdr;
    printf("check mCastAdr = %d.%d.%d.%d\n", *(prtPtr + 0), *(prtPtr + 1),  *(prtPtr + 2), *(prtPtr + 3));
    printf("mCastAdr=%8x, multiStart=%8x\n", hl2net(*(uint32_t*)mCastAdr), hl2net(*(uint32_t*)multiStart));
#endif
    if (hl2net(*(uint32_t*)mCastAdr) >= hl2net(*(uint32_t*)multiStart))
    {
#ifdef debug
        printf("mCastAdr=%8x, multiEnd=%8x\n", hl2net(*(uint32_t*)mCastAdr), hl2net(*(uint32_t*)multiEnd));
#endif
        if (hl2net(*(uint32_t*)mCastAdr) <= hl2net(*(uint32_t*)multiEnd))
        {
            res = 1;                                            /* true:no problem */
        }
    }
    if (res == 0)
    {
#ifdef debug
        printf("mCastAdr out of range\n");
#endif
    }
    return res;
}

/***********************************************************************************************************************
* Function Name: state_not_non_member_search
* Description  : state_not_non_member_search
* Arguments    : mCastAdr
*              :  ip address work pointer
*              : RJ45port
*              :  interface
* Return Value : IGMP_GROUP_ARRAY
*              :  terget work pointer
*              : 0
*              :  nothing
***********************************************************************************************************************/
static IGMP_GROUP_ARRAY* state_not_non_member_search(UB* mCastAdr, UW RJ45port)
{
    IGMP_GROUP_ARRAY *igmpPt = IGMP_WK;

    do
    {
        if (igmpPt->state != ENM_NON_MEMBER)
        {
            if (_cmp_ipaddr(igmpPt->mCastAdr, mCastAdr) == 0 && igmpPt->RJ45port == RJ45port)   /* match */
            {
                return igmpPt;                                  /* terget address return */
            }
        }
    }
    while (++igmpPt < &IGMP_WK[MULTI_CAST_MAX_GROUP_COUNT]);    /* WAIT_LOOP */
    return (void*)0;                                            /* not match */
}

/***********************************************************************************************************************
* Function Name: state_non_member_search
* Description  : state_non_member_search
* Arguments    : mCastAdr
*              :  ip address work pointer
* Return Value : IGMP_GROUP_ARRAY
*              :  terget work pointer
*              : 0
*              :  nothing
***********************************************************************************************************************/
static IGMP_GROUP_ARRAY* state_non_member_search(UB* mCastAdr)
{
    IGMP_GROUP_ARRAY *igmpPt = IGMP_WK;

    do
    {
        if (igmpPt->state == ENM_NON_MEMBER)                    /* blank area */
        {
            return igmpPt;                                      /* blank work address return */
        }
    }
    while (++igmpPt < &IGMP_WK[MULTI_CAST_MAX_GROUP_COUNT]);
    return (void*)0;                                            /* not blank */
}


/***********************************************************************************************************************
* Function Name: igmp_ver_change
* Description  : igmp_ver_change
* Arguments    : settingPt
*              :  work pointer
* Return Value : none
***********************************************************************************************************************/
static void igmp_ver_change(IGMP_GROUP_ARRAY *settingPt)
{
    if ((settingPt->igmpModeReq) && (settingPt->igmpMode != settingPt->igmpModeReq))
    {
#ifdef debug
        printf("IGMPv%d -> IGMPv%d chg\n", settingPt->igmpMode, settingPt->igmpModeReq);
#endif
        settingPt->igmpMode = settingPt->igmpModeReq;           /* IGMP version change */
    }
}


/***********************************************************************************************************************
* Function Name: make_igmp_report_header
* Description  : make_igmp_report_header
* Arguments    : settingPt
*              :  work pointer
*              : sendPacketImg
*              :  data pointer
* Return Value : none
***********************************************************************************************************************/
static void make_igmp_report_header(IGMP_PACKET_EX *sendPacketPt, IGMP_GROUP_ARRAY *settingPt)
{
    IGMP_PACKET     *caseIGMPv1Type;

    make_igmp_etherLayerHeader(sendPacketPt, settingPt);
    make_igmp_IPv4LayerHeader(sendPacketPt, settingPt);

    if (settingPt->igmpMode == ENM_IGMPV1_TYPE)                 /* case IGMPv1 */
    {
        caseIGMPv1Type = (IGMP_PACKET*)sendPacketPt;            /* Remove the option area */
        caseIGMPv1Type->igmpLay.vertype = ENM_IGMP_REPORT_V1;
        memcpy(&(caseIGMPv1Type->igmpLay.group), settingPt->mCastAdr, IP_ALEN);
        caseIGMPv1Type->igmpLay.igmpcksum = hs2net(_cksum((uchar *) &
                                            (caseIGMPv1Type->igmpLay), sizeof(IGMP_HEADER), 0));
    }
    else if (settingPt->igmpMode == ENM_IGMPV2_TYPE)            /* case IGMPv2 */
    {
        sendPacketPt->igmpLay.vertype = ENM_IGMP_REPORT_V2;
        memcpy(&(sendPacketPt->igmpLay.group), settingPt->mCastAdr, IP_ALEN);
        sendPacketPt->igmpLay.igmpcksum = hs2net(_cksum((uchar *) & (sendPacketPt->igmpLay), sizeof(IGMP_HEADER), 0));
    }
    else
    {
        /*nothing to do*/
    }
}

#ifdef debug
/***********************************************************************************************************************
* Function Name: debug_print_for_send_packet
* Description  : debug_print_for_send_packet
* Arguments    : settingPt
*              :  work pointer
*              : sendPacketImg
*              :  data pointer
*              : sendSiz
*              :  send size
* Return Value : none
***********************************************************************************************************************/
void debug_print_for_send_packet(IGMP_GROUP_ARRAY *settingPt, IGMP_PACKET_EX sendPacketImg, uint32_t sendSiz)
{
    unsigned char *prtPtr;
    int i;

    if (settingPt->igmpMode == ENM_IGMPV2_TYPE)
    {
        prtPtr = (unsigned char *) & sendPacketImg.igmpLay.group;
    }
    else
    {
        prtPtr = ((IGMP_PACKET*)(&sendPacketImg))->igmpLay.group;
    }
    printf("mCastAdr = %d.%d.%d.%d\n", *(prtPtr + 0), *(prtPtr + 1),  *(prtPtr + 2), *(prtPtr + 3));
    printf("EthLay(%d):\n", sizeof(_ETH_HDR));
    prtPtr = (unsigned char *) & sendPacketImg.etherLay;
    /* WAIT_LOOP */
    for (i = 0; i < sizeof(_ETH_HDR); i++)
    {
        printf("%02x ", *prtPtr++);
    }
    printf("\n");
    prtPtr = (unsigned char *) & sendPacketImg.ipv4Lay;
    printf("rest(%d):\n", sendSiz);
    /* WAIT_LOOP */
    for (i = 0; i < sendSiz; i++)
    {
        if (sendSiz > 28)
        {
            if (i == 20)
            {
                printf("\noption>");
            }
            if (i == 24)
            {
                printf(">\n");
            }
        }
        else
        {
            if (i == 20)
            {
                printf("\n");
            }
        }
        printf("%02x ", *prtPtr++);
    }
    printf("\n\n");
}
#endif

/***********************************************************************************************************************
* Function Name: send_igmp_report
* Description  : send_igmp_report
* Arguments    : settingPt
*              :  work pointer
* Return Value : -1
*              :  fail
*              : E_OK
*              :  success
***********************************************************************************************************************/
static int32_t send_igmp_report(IGMP_GROUP_ARRAY *settingPt)
{
    IGMP_PACKET_EX  sendPacketImg;
    int32_t         res = -1;
    int32_t         sendSiz = 0;

    igmp_ver_change(settingPt);
    memset(&sendPacketImg, 0, sizeof(IGMP_PACKET_EX));              /* send packet area ZERO reset */
    make_igmp_report_header(&sendPacketImg, settingPt);             /* header make */
    if (settingPt->igmpMode == ENM_IGMPV1_TYPE)
    {
        sendSiz = (sizeof(_IP_HDR) + sizeof(IGMP_HEADER));          /* case IP layer option not use */
    }
    else if (settingPt->igmpMode == ENM_IGMPV2_TYPE)
    {
        sendSiz = (sizeof(_IP_HDR_EX) + sizeof(IGMP_HEADER));       /* case IP layer option not use */
    }
    else
    {
        /* nothing to do */
    }
    res = lan_write(settingPt->RJ45port,
                    (B *) & (sendPacketImg.etherLay), (H)sizeof(_ETH_HDR), (B *) & (sendPacketImg.ipv4Lay), (H)sendSiz);

#ifdef debug
    debug_print_for_send_packet(settingPt, sendPacketImg, sendSiz);
    printf(">>>>>lan_write = %ld>>>>>\n\n", res);
#endif
    return res;
}

/***********************************************************************************************************************
* Function Name: make_igmp_leave_header
* Description  : make_igmp_leave_header
* Arguments    : sendPacketPt
*              :  data pointer
*              : settingPt
*              :  work pointer
* Return Value : none
***********************************************************************************************************************/
static void make_igmp_leave_header(IGMP_PACKET_EX *sendPacketPt, IGMP_GROUP_ARRAY *settingPt)       /* IGMPv2 only */
{
    make_igmp_etherLayerHeader(sendPacketPt, settingPt);
    make_igmp_IPv4LayerHeader(sendPacketPt, settingPt);
    sendPacketPt->igmpLay.vertype = ENM_IGMP_LEAVE_GROUP;
    memcpy(&(sendPacketPt->igmpLay.group), settingPt->mCastAdr, IP_ALEN);
    sendPacketPt->igmpLay.igmpcksum = hs2net(_cksum((uchar *) & (sendPacketPt->igmpLay), sizeof(IGMP_HEADER), 0));
}

/***********************************************************************************************************************
* Function Name: send_igmp_leave
* Description  : send_igmp_leave
* Arguments    : settingPt
*              :  work pointer
* Return Value : res
*              :  lan_write() return
***********************************************************************************************************************/
static int32_t send_igmp_leave(IGMP_GROUP_ARRAY *settingPt)
{
    IGMP_PACKET_EX sendPacketImg;
    int32_t     res = 0;
    int32_t     sendSiz;

    igmp_ver_change(settingPt);
    memset(&sendPacketImg, 0, sizeof(IGMP_PACKET_EX));              /* send packet area ZERO reset */
    make_igmp_leave_header(&sendPacketImg, settingPt);              /* header make */

    sendSiz = (sizeof(_IP_HDR_EX) + sizeof(IGMP_HEADER));           /* case IP layer option not use */
    res = lan_write(settingPt->RJ45port,
                    (B *) & (sendPacketImg.etherLay), sizeof(_ETH_HDR), (B *) & (sendPacketImg.ipv4Lay), sendSiz);
#ifdef debug
    debug_print_for_send_packet(settingPt, sendPacketImg, sendSiz);
    printf(">>>>>lan_write = %ld>>>>>\n\n", res);
#endif
    return res;
}



/***********************************************************************************************************************
* Function Name: igmp_join_group
* Description  : igmp_join_group
* Arguments    : mCastAdr
*              :   ip address work pointer
*              : RJ45port
*              :   interface
* Return Value : E_IGMP_MULTICAST_OUT_OF_RANGE
*              :  out of range
*              : E_IGMP_MULTICAST_MAX_ENTRY
*              :  not entry
*              : E_IGMP_MULTICAST_DOUBLE_ENTRY
*              :  duplicate entry
*              : E_OK
*              :  success
***********************************************************************************************************************/
UW igmp_join_group(UW* mCastAdr, UW RJ45port)
{
    int32_t res = E_IGMP_MULTICAST_OUT_OF_RANGE;
    IGMP_GROUP_ARRAY *settingPt;
    uint32_t    rndWk;

#ifdef debug
    printf("JoinReq\n");
#endif
    if (igmp_multicast_address_check((UB*)mCastAdr) == 1)           /* Multicast range check and correct decision */
    {
        dis_int();
        if (state_not_non_member_search((UB*)mCastAdr, RJ45port) == (void*)0) /* ZERO is not same address(Newregist) */
        {
            settingPt = state_non_member_search((UB*)mCastAdr);     /* Free space search */
            if (settingPt != (void*)0)                              /* return free space ptr. */
            {
                if(0 ==  _ip_available_check(RJ45port))
                {
#ifdef debug
                    printf("--send_igmp_join_report--\n");
#endif
                    settingPt->igmpMode = INITIAL_IGMP_VER;             /* IGMP default version */
                    settingPt->state = ENM_DELAYING_MEMBER;             /* RFC designation */
                    settingPt->RJ45port = RJ45port;
                    settingPt->finalAnswer = 1;                         /* set flag. RFC designation */
                    memcpy(settingPt->mCastAdr, mCastAdr, IP_ALEN);     /* multicast addrtess entry */
                    res = send_igmp_report(settingPt);                  /* send packet */
                    get_random_number((UB*)&rndWk, 4);
                    settingPt->repDelayTimer = (UH)((rndWk ^ (uint32_t)(settingPt->mCastAdr)) % C10SEC);
#ifdef debug
                    printf("--repDelayTimer(%ld)--\n", settingPt->repDelayTimer);
#endif
                }
                else
                {
                    res = E_IGMP_SYSTEM_ERROR;                          /* IP address is not confirm */
#ifdef debug
                    printf("IP address is not confirm\n");
#endif
                }
            }
            else
            {
                res = E_IGMP_MULTICAST_MAX_ENTRY;                   /* cant register any more */
#ifdef debug
                printf("max entry\n", settingPt->repDelayTimer);
#endif
            }
        }
        else
        {
            res = E_IGMP_MULTICAST_DOUBLE_ENTRY;                    /* same address detect */
#ifdef debug
            printf("mCastAdr double entry\n", settingPt->repDelayTimer);
#endif
        }
        ena_int();
    }
    else
    {
        res = E_IGMP_MULTICAST_OUT_OF_RANGE;                        /* out of range */
#ifdef debug
        printf("illegal mCastAdr out of range\n", settingPt->repDelayTimer);
#endif
    }
    return res;
}


/***********************************************************************************************************************
* Function Name: int_send_igmp_when_router_query_receive
* Description  : int_send_igmp_when_router_query_receive
* Arguments    : RJ45port
*              :  interface
*              : rxIPlayDatPt
*              :  data
* Return Value : none
***********************************************************************************************************************/
void int_send_igmp_when_router_query_receive(uint8_t RJ45port, _P_RCV_BUF *rxIPlayDatPt)
{
    IGMP_GROUP_ARRAY *settingPt = IGMP_WK;
    _IP_HDR     *ipdatPt;
    IGMP_HEADER *igmpdatPt;
    uint16_t    calc_chksum;
    uint32_t    verJudgeV2 = 0;
    uint32_t    untreated = 1;                                   /* 1 = Untreated */
    uint32_t    rndWk;
    uint32_t    ipHdrSiz;
    uint32_t    igmpHdrSiz;
    UB          timer;

#ifdef debug
    printf("--rcv_igmp_Query--\n");
#endif
    ipdatPt = (_IP_HDR *)(rxIPlayDatPt->pip);                    /* ipLayerHeader */
    ipHdrSiz = ((*(uchar*)ipdatPt) & 0xf) * 4;
    igmpdatPt = (IGMP_HEADER*)((uchar*)ipdatPt + ipHdrSiz);      /* ip header skip, get igmp header */
    igmpHdrSiz = net2hs(ipdatPt->ip_total_len) - ipHdrSiz;

    if (igmpdatPt->time.maxRespTime)                             /* IGMP header maxRespTime is value != 0 */
    {
        verJudgeV2 = 1;                                          /* IGMPv2 */
    }
    if (igmpdatPt->vertype != ENM_IGMP_QUERY)                    /* 0x11(Query)? */
    {
#ifdef debug
        printf("--but rcv_membershipReport--\n");
#endif
        int_send_igmp_when_membership_report_receive(RJ45port, rxIPlayDatPt);       /* not Query, Think memberReport */
        return;
    }
#ifdef debug
    if (igmpHdrSiz > sizeof(IGMP_HEADER))
    {
        printf("IGMP v3 header!!\n");
    }
    printf("maxRespTime=%d,IGMPv%d\n", igmpdatPt->time.maxRespTime, verJudgeV2 + 1);
#endif

    /* IGMP layer check block */
    calc_chksum = hs2net(_cksum((uchar *)igmpdatPt, igmpHdrSiz, 0));
    if (calc_chksum != 0)                       /* checksum same check */
    {
#ifdef debug
        printf("checksum error. drop packet\n\n");
#endif
        report_error(RJ45port, RE_IGMP_HEADER1, data_link_buf_ptr);
        return;
    }
    else
    {
#ifdef debug
        printf("no problem packet(checksum OK)\n");
#endif
    }

    do
    {
        if (settingPt->RJ45port == (uint32_t)RJ45port)
        {
            if (settingPt->state == ENM_IDLE_MEMBER)
            {
#ifdef debug
                uint8_t *bPt = (uint8_t*)settingPt->mCastAdr;
                printf("--IdleMember(%3d,%3d,%3d%3d),RJ45_(%d)--\n",
                       *bPt, *(bPt + 1), *(bPt + 2), *(bPt + 3), settingPt->RJ45port);
#endif
                if (settingPt->igmpMode == ENM_IGMPV1_TYPE && verJudgeV2 == 1)          /* I IGMPv1 you IGMPv2 */
                {
                    settingPt->igmpModeReq = ENM_IGMPV2_TYPE;        /* ver change REQUEST */
#ifdef debug
                    printf("IGMPv1 -> IGMPv2 chg req\n");
#endif
                }
                else if (settingPt->igmpMode == ENM_IGMPV2_TYPE && verJudgeV2 == 0)     /* I IGMPv2 you IGMPv1 */
                {
                    settingPt->igmpModeReq = ENM_IGMPV1_TYPE;        /* ver change request */
#ifdef debug
                    printf("IGMPv2 -> IGMPv1Type chg req\n");
#endif
                }
                if (settingPt->igmpMode == ENM_IGMPV2_TYPE)          /* i mode is IGMPv2 */
                {
                    if ((verJudgeV2 == 1) && (*(uint32_t*)(settingPt->mCastAdr) != *(uint32_t*)(igmpdatPt->group)))
                    {
                        if (*(uint32_t*)(igmpdatPt->group) != 0)     /* not general Query */
                        {
                            continue;                                /* dont process, specific address */
                        }
                    }
                    else                                             /* general query(including IGMPv1 packet) */
                    {
                        /* nothing to do */
                    }
                    timer = igmpdatPt->time.maxRespTime;             /* for Query */
                    if (timer == 0)
                    {
                        get_random_number((UB*)&rndWk, 4);
                        settingPt->repDelayTimer = (UH)((rndWk ^ (uint32_t)(settingPt->mCastAdr)) % C10SEC);
#ifdef debug
                        printf("IdleMember:IGMPv2:--delay_timer_set,RND(%d)=(%ld)--\n", \
                               C10SEC , settingPt->repDelayTimer);
#endif
                    }
                    else
                    {
                        get_random_number((UB*)&rndWk, 4);
                        settingPt->repDelayTimer = (UH)((rndWk ^ (uint32_t)(settingPt->mCastAdr)) % (timer * 10));
#ifdef debug
                        printf("IdleMember:IGMPv2:--delay_timer_set,RND(%d)=(%ld)--\n",
                               timer * 10, settingPt->repDelayTimer);
#endif
                    }
                }
                else                                                                    /* i mode is IGMPv1 */
                {
                    get_random_number((UB*)&rndWk, 4);
                    settingPt->repDelayTimer = (UH)((rndWk ^ (uint32_t)(settingPt->mCastAdr)) % C10SEC);
#ifdef debug
                    printf("IdleMember:IGMPv1:--delay_timer_set,RND(%d)=(%ld)--\n", C10SEC, settingPt->repDelayTimer);
#endif
                }
                settingPt->state = ENM_DELAYING_MEMBER;
            }
            else if (settingPt->state == ENM_DELAYING_MEMBER)
            {
#ifdef debug
                uint8_t *bPt = (uint8_t*)settingPt->mCastAdr;
                printf("--DelayingMember(%3d,%3d,%3d%3d),RJ45_(%d)--\n",
                       *bPt, *(bPt + 1), *(bPt + 2), *(bPt + 3), settingPt->RJ45port);
#endif
                if (settingPt->igmpMode == ENM_IGMPV2_TYPE)                             /* i mode is IGMPv2 */
                {
                    if (settingPt->repDelayTimer > C10SEC)
                    {
                        get_random_number((UB*)&rndWk, 4);
                        settingPt->repDelayTimer = (UH)((rndWk ^ (uint32_t)(settingPt->mCastAdr)) % C10SEC);
#ifdef debug
                        printf("delayingMember:--delay_timer_re-set,RND(%d)=(%ld)--\n",
                               C10SEC , settingPt->repDelayTimer);
#endif
                    }
                }
            }
            else        /* idle member */
            {
#ifdef debug
                printf("--NonMember--\n");
#endif
                /* nothing to do */
            }
            untreated = 0;                                                          /* treated=0, untreated=1 */
        }
    }
    while (++settingPt < &IGMP_WK[MULTI_CAST_MAX_GROUP_COUNT]);    /* WAIT_LOOP */
    if (untreated == 1)
    {
#ifdef debug
        printf("--drop query--\n");
#endif
        report_error(RJ45port, RE_IGMP_HEADER2, data_link_buf_ptr);
    }
}


/***********************************************************************************************************************
* Function Name: int_send_igmp_when_membership_report_receive
* Description  : int_send_igmp_when_membership_report_receive
* Arguments    : RJ45port
*              :  interface
*              : rxIPlayDatPt
*              :  data
* Return Value : none
***********************************************************************************************************************/
void int_send_igmp_when_membership_report_receive(uint8_t RJ45port, _P_RCV_BUF *rxIPlayDatPt)
{
    IGMP_GROUP_ARRAY *settingPt = IGMP_WK;
    _IP_HDR     *ipdatPt;
    IGMP_HEADER *igmpdatPt;
    uint16_t    calc_chksum;
    uint32_t    untreated = 1;                                                 /* untreated */
    uint32_t    ipHdrSiz;
    uint32_t    igmpHdrSiz;
#ifdef debug
    uint8_t *bPt;
#endif

#ifdef debug
    printf("---something multicast packet received---\n");
#endif
    ipdatPt = (_IP_HDR *)(rxIPlayDatPt->pip);                                  /* ipLayerHeader */
    ipHdrSiz = ((*(uchar*)ipdatPt) & 0xf) * 4;
    igmpdatPt = (IGMP_HEADER*)((uchar*)ipdatPt + ipHdrSiz);                    /* ip header skip, get igmp header */
    igmpHdrSiz = net2hs(ipdatPt->ip_total_len) - ipHdrSiz;

    /* IGMP VerType check part */
    if ((igmpdatPt->vertype != ENM_IGMP_REPORT_V1) && (igmpdatPt->vertype != ENM_IGMP_REPORT_V2))
    {
#ifdef debug
        printf("other Packet type. drop packet(%X)\n\n", (igmpdatPt->vertype));
#endif
        report_error(RJ45port, RE_IGMP_HEADER2, data_link_buf_ptr);
        return;
    }
#ifdef debug
    if (igmpHdrSiz > sizeof(IGMP_HEADER))
    {
        printf("IGMP v3 header!!\n");
    }
#endif

#ifdef debug
    printf("IPlay first byte = %02X\n", *(uchar*)ipdatPt);
    bPt = ipdatPt->ip_src;
    printf("IPlay src adr    = %3d,%3d,%3d,%3d\n", *bPt, *(bPt + 1), *(bPt + 2), *(bPt + 3));
    bPt = ipdatPt->ip_dst;
    printf("IPlay dst adr    = %3d,%3d,%3d,%3d\n", *bPt, *(bPt + 1), *(bPt + 2), *(bPt + 3));
    printf("igmp header verType = %02X\n", igmpdatPt->vertype);
    printf("igmp header time    = %ld\n", igmpdatPt->time);
    printf("igmp header checksum= %04X\n", *(UH*)igmpdatPt->igmpcksum);
    bPt = (UB*)igmpdatPt->group;
    printf("igmp header group   = %3d,%3d,%3d,%3d\n", *bPt, *(bPt + 1), *(bPt + 2), *(bPt + 3));
#endif
    /* IGMP layer check block */
    calc_chksum = hs2net(_cksum((uchar *)igmpdatPt, igmpHdrSiz, 0));
    if (calc_chksum != 0)                                                           /* checksum same check */
    {
#ifdef debug
        printf("checksum error. drop packet\n\n");
#endif
        report_error(RJ45port, RE_IGMP_HEADER1, data_link_buf_ptr);
        return;
    }
    else
    {
#ifdef debug
        printf("no problem packet(checksum OK)\n");
#endif
    }

    if (igmp_multicast_address_check((UB*)ipdatPt->ip_dst) == 1)
    {
        do
        {
            if (settingPt->RJ45port == (uint32_t)RJ45port)
            {
                if (settingPt->state == ENM_DELAYING_MEMBER)
                {
#ifdef debug
                    uint8_t *bPt = (uint8_t*)settingPt->mCastAdr;
                    printf("--DelayingMember(%3d,%3d,%3d%3d)--\n", *bPt, *(bPt + 1), *(bPt + 2), *(bPt + 3));
#endif
                    if (_cmp_ipaddr(settingPt->mCastAdr, ipdatPt->ip_dst) == 0)
                    {
                        if (_cmp_ipaddr(igmpdatPt->group, ipdatPt->ip_dst) == 0)
                        {
                            settingPt->repDelayTimer = 0;
                            settingPt->state = ENM_IDLE_MEMBER;
                            settingPt->finalAnswer = 0;
#ifdef debug
                            printf("--rcv_igmp_Report(otherMember)_timer_clear--\n");
#endif
                            untreated = 0;
                        }
                    }
                }
#ifdef debug
                else if (settingPt->state == ENM_IDLE_MEMBER)
                {
                    uint8_t *bPt = (uint8_t*)settingPt->mCastAdr;
                    printf("--IdleMember(%3d,%3d,%3d%3d)--\n", *bPt, *(bPt + 1), *(bPt + 2), *(bPt + 3));
                }
                else
                {
                    printf("--NonMember--\n");
                }
#endif

            }
        }
        while (++settingPt < &IGMP_WK[MULTI_CAST_MAX_GROUP_COUNT]);    /* WAIT_LOOP */
    }
    if (untreated == 1)
    {
#ifdef debug
        printf("packet untreated. drop packet\n\n");
#endif
        report_error(RJ45port, RE_IGMP_HEADER2, data_link_buf_ptr);
    }
}

/***********************************************************************************************************************
* Function Name: int_send_igmp_timer_expired
* Description  : int_send_igmp_timer_expired
* Arguments    : now_time_count
*              :  time
* Return Value : none
***********************************************************************************************************************/
void int_send_igmp_timer_expired(UH now_time_count)
{
    IGMP_GROUP_ARRAY *settingPt = IGMP_WK;

    do
    {
        if (settingPt->state == ENM_DELAYING_MEMBER)
        {
            if (settingPt->repDelayTimer != 0xFFFF)                   /* remaining timer, ZERO Countermeasure */
            {
                if (--(settingPt->repDelayTimer) == 0xFFFF)           /* count down, just limit?, ZERO Countermeasure */
                {
#ifdef debug
                    printf("--snd_member_report--\n");
#endif
                    send_igmp_report(settingPt);                      /* send membership Report */
                    settingPt->state = ENM_IDLE_MEMBER;
                    settingPt->finalAnswer = 1;
                }
            }
        }
    }
    while (++settingPt < &IGMP_WK[MULTI_CAST_MAX_GROUP_COUNT]);    /* WAIT_LOOP */
}

/***********************************************************************************************************************
* Function Name: igmp_leave_group
* Description  : igmp_leave_group
* Arguments    : mCastAdr
*              :   ip address work pointer
*              : RJ45port
*              :   interface
* Return Value : E_IGMP_MULTICAST_NOT_ENTRY
*              :  not entry
*              : E_OK
*              :  success
***********************************************************************************************************************/
UW igmp_leave_group(UW* mCastAdr, UW RJ45port)
{
    IGMP_GROUP_ARRAY *settingPt = IGMP_WK;
    int32_t res = E_IGMP_MULTICAST_NOT_ENTRY;

    dis_int();
    do
    {
        if (settingPt->state != ENM_NON_MEMBER)                        /* something to entry */
        {
            if (_cmp_ipaddr(settingPt->mCastAdr, mCastAdr) == 0 && settingPt->RJ45port == RJ45port)
            {
                if (settingPt->igmpMode == ENM_IGMPV2_TYPE)
                {
                    if (settingPt->finalAnswer)
                    {
                        if(0 ==  _ip_available_check(RJ45port))
                        {
                            settingPt->leaveFlag = 1;                      /* leave packet send distination is 224.0.0.2 */
                            res = send_igmp_leave(settingPt);
                        }
                        else
                        {
                            res = E_IGMP_SYSTEM_ERROR;                          /* IP address is not confirm */
#ifdef debug
                            printf("IP address is not confirm\n");
#endif
                        }
                    }
                    else                                               /* IGMPv2 is not send leave packet */
                    {
                        res = E_OK;
#ifdef debug
                        printf("IGMPv2 not last member dont send leave packet\n");
#endif
                    }
                }
                else
                {
                    res = E_OK;
#ifdef debug
                    printf("IGMPv1 dont send leave packet(RFC1112 compliance)\n");
#endif
                }
                memset(settingPt, 0, sizeof(IGMP_GROUP_ARRAY));
                ena_int();
                return res;
            }
        }
    }
    while (++settingPt < &IGMP_WK[MULTI_CAST_MAX_GROUP_COUNT]);    /* WAIT_LOOP */
    ena_int();
    return res;
}

