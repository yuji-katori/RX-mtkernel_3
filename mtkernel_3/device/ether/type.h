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
* File Name    : type.h
* Version      : 2.10
* Description  : standard type for T4 header file
* Website      : https://www.renesas.com/mw/t4
***********************************************************************************************************************/
/***********************************************************************************************************************
* History : DD.MM.YYYY Version Description
*         : 01.04.2014 1.00    First Release
*         : 30.11.2016 1.01    File Header maintenance
*         : 20.06.2019 2.09    Added support GCC RX compiler and IAR RX compiler.
***********************************************************************************************************************/
#ifndef __TYPE_H__
#define __TYPE_H__

#define _TCP_UNIT_TIME  10    /* 10ms */

typedef   signed char  schar;
typedef unsigned char  uchar;
typedef   signed short  sint16;
typedef unsigned short  uint16;
typedef   signed long  sint32;
typedef unsigned long  uint32;

#define TRUE  1
#define FALSE  0

/* Little Endian */
#if defined(R8C) || defined(M16C) || defined(M16C80) || defined(M32C80) ||\
 (defined(_SH4) && defined(_LIT)) ||\
 (defined(_SH4A) && defined(_LIT)) ||\
 (defined(__RX) && defined(__LIT)) ||\
    defined(__v850) ||\
    (defined(__GNUC__)  && (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)) ||\
    (defined(__ICCRX__) && defined(__LIT))
#define  BIGENDIAN 0

/* Big Endian */
#elif defined(__300HA__) || defined (__2600A__) ||\
   defined(_SH2) || defined(_SH2A) || defined(_SH2AFPU) ||\
   (defined(_SH4) && defined(_BIG)) ||\
   (defined(_SH4A) && defined(_BIG)) ||\
   (defined(__RX) && defined(__BIG)) ||\
   (defined(__GNUC__) && (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)) ||\
   (defined(__ICCRX__) && defined(__BIG))
#define  BIGENDIAN 1
#endif

/* near/far definition */
#if !defined(R8C) || !defined(M16C) || !defined(M16C80) || !defined(M32C80)
#define  far
#define  near
#endif

#if BIGENDIAN == 1
#define type_h_define_BIGENDIAN_detect
#define hs2net(x) (x)
#define net2hs(x) (x)
#define hl2net(x) (x)
#define net2hl(x) (x)
void net2hl_yn_xn(void *y, void *x);
#define hl2net_yn_xn(y, x) net2hl_yn_xn(y, x)

#else /* BIGENDIAN == 0 */

#define hs2net(x) (uint16_t)(((uint16)(x)>>8)  | ((uint16)(x)<<8))
#define net2hs(x) (uint16_t)(((uint16)(x)>>8)  | ((uint16)(x)<<8))
#define hl2net(x) ((uint32)((0xff000000&(x))>>24) | (uint32)((0x00ff0000&(x))>>8) | \
                   (uint32)((0x0000ff00&(x))<<8) | (uint32)((0x000000ff&(x))<<24))

#define net2hl(x) ((uint32)((0xff000000&(x))>>24) | (uint32)((0x00ff0000&(x))>>8) | \
                   (uint32)((0x0000ff00&(x))<<8) | (uint32)((0x000000ff&(x))<<24))

void net2hl_yn_xn(void *y, void *x);
#define hl2net_yn_xn(y, x) net2hl_yn_xn(y, x)
#define net2hl_xn_xn(x)\
    {\
        register tmp1; \
        register tmp2; \
        register uchar *a0 = (uchar *)x; \
        tmp1 = *((uchar*)a0    );\
        tmp2 = *((uchar*)a0 + 3);\
        *((uchar*)a0    ) = tmp2;\
        *((uchar*)a0 +3 ) = tmp1;\
        tmp1 = *((uchar*)a0 + 1);\
        tmp2 = *((uchar*)a0 + 2);\
        *((uchar*)a0 + 1) = tmp2;\
        *((uchar*)a0 + 2) = tmp1;\
    }
#endif

#if !defined(NULL)
#if defined(M16C) || defined(M16C80) || defined(M32C80) || defined(R8C)
#define NULL  0
#endif
#endif

#define T_IPVxEP T_IPV4EP

#define EP_ALEN    6
typedef uchar Eaddr[EP_ALEN];  /*  MAC address */

/*
 * Macro function
 */
#if defined(M16C) || defined(M16C80) || defined(M32C80) || defined(R8C) ||\
 defined(__300HA__) || defined(__2600A__) ||\
 defined(_SH2) || defined(_SH2A) || defined(_SH2AFPU) || defined(_SH4) || defined(_SH4A) ||\
 defined(__RX) || defined(__v850) ||\
 (defined(__GNUC__) && defined(__RX__)) ||\
 defined(__ICCRX__)
/* same as memcmp(x, y, IP_ALEN) */
#if defined (_SH2) || defined(_SH2A) || defined(_SH2AFPU) || defined(_SH4) || defined(_SH4A) ||\
 defined(__RX) || defined(__v850) ||\
 (defined(__GNUC__) && defined(__RX__)) ||\
 defined(__ICCRX__)
#define _cmp_ipaddr(x,y) memcmp(x,y,(size_t)IP_ALEN)
#else
#define _cmp_ipaddr(x, y)  ( ((*(((uint16 *)(x))  )) ^ (*(((uint16 *)(y))  ))) |  \
                             ((*(((uint16 *)(x))+1)) ^ (*(((uint16 *)(y))+1)))  )
#endif
/* same as memcpy(x, y, IP_ALEN) */
#if defined (_SH2) || defined(_SH2A) || defined(_SH2AFPU) || defined(_SH4) || defined(_SH4A) ||\
 defined(__RX) || defined(__v850) ||\
 (defined(__GNUC__) && defined(__RX__)) ||\
 defined(__ICCRX__)
#define _cpy_ipaddr(x ,y) memcpy(x,y,IP_ALEN)
#else
#define _cpy_ipaddr(x, y)  *((uint32 *)(x)) = *((uint32 *)(y))
#endif
/* same as memcpy(x, y, EP_ALEN) */
#if defined (_SH2) || defined(_SH2A) || defined(_SH2AFPU) || defined(_SH4) || defined(_SH4A) ||\
 defined(__RX) || defined(__v850) ||\
 (defined(__GNUC__) && defined(__RX__)) ||\
 defined(__ICCRX__)
#define _cpy_eaddr(x, y) memcpy(x,y,EP_ALEN);
#else
#define _cpy_eaddr(x, y)   *(((uint32 *)(x))  ) = *(((uint32 *)(y))  ); \
    *(((uint16 *)(x))+2) = *(((uint16 *)(y))+2);
#endif
#define _cmp_ipv6addr(x, y)   ( ((*(((uint32 *)(x))  )) ^ (*(((uint32 *)(y))  ))) |  \
                                ((*(((uint32 *)(x))+1)) ^ (*(((uint32 *)(y))+1))) |  \
                                ((*(((uint32 *)(x))+2)) ^ (*(((uint32 *)(y))+2))) |  \
                                ((*(((uint32 *)(x))+3)) ^ (*(((uint32 *)(y))+3)))  )
#define _cpy_ipv6addr(x, y)  *(((uint32 *)(x))  ) = *(((uint32 *)(y))  ); \
    *(((uint32 *)(x))+1) = *(((uint32 *)(y))+1); \
    *(((uint32 *)(x))+2) = *(((uint32 *)(y))+2); \
    *(((uint32 *)(x))+3) = *(((uint32 *)(y))+3);
#endif



#if defined(M16C) || defined(M16C80) || defined(M32C80) || defined(R8C)
# if defined(_EN_SB)
#pragma SBDATA  _p_rcv_buf
#pragma SBDATA _myipaddr
#pragma SBDATA _ip_id
#pragma SBDATA  _tcp_timer_cnt
#pragma SBDATA  _tcp_pre_timer_cnt
#pragma SBDATA  _rcvd
#pragma SBDATA  _tcp_tcb
#pragma SBDATA _tx_hdr
#  if defined(_ETHER)
#pragma SBDATA _myethaddr
#  elif defined(_PPP)
#pragma SBDATA  ppp_info
#pragma SBDATA  ppp_sio_status
#pragma SBDATA  _ppp_status
#pragma SBDATA  _ppp_seq
#pragma SBDATA _ppp_restart
#pragma SBDATA _ppp_nak_cnt
#pragma SBDATA _ppp_auth_flag
#pragma SBDATA _ppp_api_req
#pragma SBDATA _ppp_conf
#  endif
# endif
#endif

#endif /*include guard*/






