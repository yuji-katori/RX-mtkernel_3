/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	ether_driver.c
 */

#include <string.h>
#include <stdarg.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "dev_ether.h"
#include "platform.h"
#include "ether_driver.h"
#include "r_ether_rx_if.h"

/******************************************************************************
Macro definitions
******************************************************************************/
#define	ETHER_BUFSIZE_MIN		(60)

#define	MAX_TCPCEP_CNT			(8)
#define	MAX_UDPCEP_CNT			(8)

#define	IP_TYPE_TCP			(0)
#define	IP_TYPE_UDP			(1)

#define	R_T4_DRIVER_RX_HASH_LENGTH	(32)
/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/
IMPORT UB _myethaddr[][6];
IMPORT const H __tcpcepn;
IMPORT const H __udpcepn;
IMPORT W callback_ether_regist(void);
IMPORT ID ether_objid[];
/******************************************************************************
Private global variables and functions
******************************************************************************/
LOCAL UH tcpudp_time_cnt;
LOCAL T4_STATISTICS t4_stat[CFG_SYSTEM_CHANNEL_NUMBER];

LOCAL UW tcp_tsk_tbl[MAX_TCPCEP_CNT];
LOCAL UW udp_tsk_tbl[MAX_UDPCEP_CNT];

LOCAL ER sleep_task(ID cepid, BOOL ip_type);
LOCAL ER wakeup_task(ID cepid, BOOL ip_type);
/******************************************************************************
Functions (API)
******************************************************************************/
EXPORT ER lan_open(void)
{
ER ret;
ether_param_t param;

	R_ETHER_Initial( );
	Initialize_Ether( );
	callback_ether_regist( );

	param.channel = ETHER_CHANNEL_0;
	R_ETHER_Control( CONTROL_POWER_ON, param );
#if (ETHER_CHANNEL_MAX >= 2)
	param.channel = ETHER_CHANNEL_1;
	R_ETHER_Control( CONTROL_POWER_ON, param );
#endif

	memset( t4_stat, 0, sizeof(T4_STATISTICS) );
	ret = R_ETHER_Open_ZC2( 0, _myethaddr[0], false );
	if( ETHER_SUCCESS != ret )
		return -1;
#if (ETHER_CHANNEL_MAX >= 2)
	ret = R_ETHER_Open_ZC2( 1, _myethaddr[1], false );
	if( ETHER_SUCCESS != ret )
		return -1;
#endif
	return E_OK;
}

EXPORT ER lan_close(void)
{
ether_param_t param;

	R_ETHER_Close_ZC2( 0 );
#if (ETHER_CHANNEL_MAX >= 2)
	R_ETHER_Close_ZC2( 1 );
#endif
	param.channel = ETHER_CHANNEL_0;
	R_ETHER_Control( CONTROL_POWER_OFF, param );
#if (ETHER_CHANNEL_MAX >= 2)
	param.channel = ETHER_CHANNEL_1;
	R_ETHER_Control( CONTROL_POWER_OFF, param );
#endif
	return E_OK;
}
/******************************************************************************
Functions (Use definition function that called from T4 library)
******************************************************************************/
EXPORT H rcv_buff_release(UB lan_port_no)
{
	/* This function is called when TCP/IP finished using receive buffer specified lan_read. */
	R_ETHER_Read_ZC2_BufRelease( lan_port_no );
	return 0;
}

EXPORT UH tcpudp_get_time(void)
{
	return tcpudp_time_cnt;
}

EXPORT void lan_reset(UB lan_port_no)
{
ether_param_t param;

	param.channel = lan_port_no;
	R_ETHER_Close_ZC2( lan_port_no );
	R_ETHER_Control( CONTROL_POWER_OFF, param );
	R_ETHER_Control( CONTROL_POWER_ON,  param );
	R_ETHER_Open_ZC2( lan_port_no, _myethaddr[lan_port_no], false );
}

EXPORT void udp_api_slp(ID cepid)
{
	/* If user uses "Real time OS", user may define "sleep task" here. */
	if( sleep_task( cepid, IP_TYPE_UDP ) != E_OK )
		while( 1 )  ;
}

EXPORT void udp_api_wup(ID cepid)
{
	/* If user uses "Real time OS", user may define "wake up task" here. */
	if( wakeup_task( cepid, IP_TYPE_UDP ) != E_OK )
		while( 1 )  ;
}

EXPORT void tcp_api_slp(ID cepid)
{
	/* If user uses "Real time OS", user may define "sleep task" here. */
	if( sleep_task( cepid, IP_TYPE_TCP ) != E_OK )
		while( 1 )  ;
}

EXPORT void tcp_api_wup(ID cepid)
{
	/* If user uses "Real time OS", user may define "wake up task" here. */
	if( wakeup_task( cepid, IP_TYPE_TCP ) != E_OK )
		while( 1 )  ;
}

EXPORT H lan_read(UB lan_port_no, B **buf)
{
W  driver_ret;
B *data;

	driver_ret = R_ETHER_Read_ZC2(lan_port_no, (void **)buf);
	if( driver_ret > 0 )  {
		data = (B *)*buf;
		if( !memcmp( &data[6], &_myethaddr[lan_port_no], 6 ) )  {
			rcv_buff_release( lan_port_no );
			return -1;
		}
		t4_stat[lan_port_no].t4_rec_cnt++;
		t4_stat[lan_port_no].t4_rec_byte += (UW)driver_ret;
		return driver_ret;
	}
	else if( !driver_ret )
		// R_Ether_Read() returns "0" when receiving data is nothing
		return -1;	// Return code "-1" notifies "no data" to T4.
	else
		// R_Ether_Read() returns "negative values" when error occurred
		return -2;	// Return code "-2" notifies "Ether controller disable" to T4.
}

EXPORT H lan_write(UB lan_port_no, B *header, H header_len, B *data, H data_len)
{
W   driver_ret;
B  *buf;
UH  buf_size, framesize;

	driver_ret = R_ETHER_Write_ZC2_GetBuf(lan_port_no, (void**)&buf, &buf_size );
	if( ETHER_SUCCESS == driver_ret )  {
	        framesize = header_len + data_len;		/*data length calc.*/
		if( buf_size >= framesize )  {
			memcpy( buf, header, header_len );
			memcpy( buf + header_len, data, data_len );
			if( framesize < ETHER_BUFSIZE_MIN )  {	/*under minimum*/
				memset( (buf + framesize), 0, (ETHER_BUFSIZE_MIN - framesize) );	/*padding*/
				framesize = ETHER_BUFSIZE_MIN;						/*resize*/
			}
			driver_ret =  R_ETHER_Write_ZC2_SetBuf( lan_port_no, framesize );
			if( ETHER_SUCCESS == driver_ret )  {
				t4_stat[lan_port_no].t4_snd_cnt++;
				t4_stat[lan_port_no].t4_snd_byte += framesize;
				return 0;
			}
		}
	}
	return -5;
}

EXPORT void report_error(UB lan_port_no, H err_code, UB *err_data)
{
	switch( err_code )  {
	case RE_LEN:
		t4_stat[lan_port_no].re_len_cnt++;
		break;
        case RE_NETWORK_LAYER:
		t4_stat[lan_port_no].re_network_layer_cnt++;
		break;
	case RE_TRANSPORT_LAYER:
		t4_stat[lan_port_no].re_transport_layer_cnt++;
		break;
	case RE_ARP_HEADER1:
		t4_stat[lan_port_no].re_arp_header1_cnt++;
		break;
	case RE_ARP_HEADER2:
		t4_stat[lan_port_no].re_arp_header2_cnt++;
		break;
	case RE_IP_HEADER1:
		t4_stat[lan_port_no].re_ip_header1_cnt++;
		break;
	case RE_IP_HEADER2:
		t4_stat[lan_port_no].re_ip_header2_cnt++;
		break;
	case RE_IP_HEADER3:
		t4_stat[lan_port_no].re_ip_header3_cnt++;
		break;
	case RE_IP_HEADER4:
		t4_stat[lan_port_no].re_ip_header4_cnt++;
		break;
	case RE_IP_HEADER5:
		t4_stat[lan_port_no].re_ip_header5_cnt++;
		break;
	case RE_IP_HEADER6:
		t4_stat[lan_port_no].re_ip_header6_cnt++;
		break;
	case RE_IP_HEADER7:
		t4_stat[lan_port_no].re_ip_header7_cnt++;
		break;
	case RE_IP_HEADER8:
		t4_stat[lan_port_no].re_ip_header8_cnt++;
		break;
	case RE_IP_HEADER9:
		t4_stat[lan_port_no].re_ip_header9_cnt++;
		break;
	case RE_TCP_HEADER1:
		t4_stat[lan_port_no].re_tcp_header1_cnt++;
		break;
	case RE_TCP_HEADER2:
		t4_stat[lan_port_no].re_tcp_header2_cnt++;
		break;
	case RE_UDP_HEADER1:
		t4_stat[lan_port_no].re_udp_header1_cnt++;
		break;
	case RE_UDP_HEADER2:
		t4_stat[lan_port_no].re_udp_header2_cnt++;
		break;
	case RE_UDP_HEADER3:
		t4_stat[lan_port_no].re_udp_header3_cnt++;
		break;
	case RE_ICMP_HEADER1:
		t4_stat[lan_port_no].re_icmp_header1_cnt++;
		break;
	case RE_IGMP_HEADER1:
		t4_stat[lan_port_no].re_igmp_header1_cnt++;
		break;
	case RE_IGMP_HEADER2:
		t4_stat[lan_port_no].re_igmp_header2_cnt++;
		break;
	case RE_DHCP_ILLEGAL:
		t4_stat[lan_port_no].re_dhcp_header1_cnt++;
		break;
	case RE_DHCP_SND_TIMEOUT:
		t4_stat[lan_port_no].re_dhcp_header2_cnt++;
		break;
	default:
		break;
	}
}
/******************************************************************************
Functions (Task and Interrupt, Cyclic handler)
******************************************************************************/
EXPORT void ether_tsk(INT stacd, void *exinf)
{
	while( 1 )  {
		tk_slp_tsk( TMO_FEVR );		// tk_slp_tsk
		_process_tcpip( );		// TCP/IP Process(T4)
	}
}

EXPORT void ether_cychdr(void *pdata)
{
	R_ETHER_LinkProcess( 0 );		// check LAN link stat
#if (ETHER_CHANNEL_MAX >= 2)
	R_ETHER_LinkProcess( 1 );
#endif
	tcpudp_time_cnt++;
	tk_wup_tsk( ether_objid[0] );		// tk_wup_tsk
}

EXPORT void lan_inthdr(void *ppram)
{
	tk_wup_tsk( ether_objid[0] );		// tk_wup_tsk
}
/******************************************************************************
Functions : random number generator
******************************************************************************/
EXPORT void get_random_number(UB *data, UW len)
{
LOCAL UW  y = 2463534242;
LOCAL UW *z = (UW *)&_myethaddr[0][2];
UW res, lp;
UB *bPtr;

	if( z != NULL )  {
		y ^= *z;
		z = NULL;
	}
	y += tcpudp_time_cnt;
	res = len / 4;
	/* WAIT_LOOP */
	for( lp=0 ; lp < res ; lp++ )  {
		y ^= (y << 13);
		y ^= (y >> 17);
		y ^= (y << 5);
		bPtr = (UB *)&y;
#if __LIT
		*((UW *)data) = ( (UW)bPtr[3] << 24 ) | ( bPtr[2] << 16 ) | ( bPtr[1] << 8 ) | bPtr[0];
#else
		*((UW *)data) = y;
#endif
		data += 4;
	}
	y ^= (y << 13);
	y ^= (y >> 17);
	y ^= (y << 5);
	res = len % 4;
	bPtr = (UB *)&y;
	switch( res )  {
	case 3:
#if __LIT
		*data++ = bPtr[3];
		*data++ = bPtr[2];
		*data++ = bPtr[1];
#else
		*data++ = bPtr[0];
		*data++ = bPtr[1];
		*data++ = bPtr[2];
#endif
		break;
	case 2:
#if __LIT
		*data++ = bPtr[3];
		*data++ = bPtr[2];
#else
		*data++ = bPtr[0];
		*data++ = bPtr[1];
#endif
		break;
	case 1:
#if __LIT
		*data++ = bPtr[3];
#else
		*data++ = bPtr[0];
#endif
		break;
	}
}
/******************************************************************************
Functions : hash value generator
            (Used SHA256 by TSIP when the MCU has TSIP, or XOR calculation for random number)
******************************************************************************/
EXPORT void get_hash_value(UB lan_port_no, UB *message, UW message_len, UB **hash, UW *hash_len)
{
LOCAL UB hash_result[ETHER_CHANNEL_MAX][R_T4_DRIVER_RX_HASH_LENGTH];

	UW hash_result_len = R_T4_DRIVER_RX_HASH_LENGTH;
	UW hash_result_index = 0;
	UW loop_cnt = 0;
	UW hash_block_cnt = message_len / R_T4_DRIVER_RX_HASH_LENGTH;
	UW hash_block_odd = message_len % R_T4_DRIVER_RX_HASH_LENGTH;
	UB random_value[R_T4_DRIVER_RX_HASH_LENGTH];

	if( ETHER_CHANNEL_MAX < lan_port_no )
		while( 1 )  ;
	memset( hash_result, 0, sizeof(hash_result) );
	if( message_len >= R_T4_DRIVER_RX_HASH_LENGTH )
		/* XOR calculation to self by R_T4_DRIVER_RX_HASH_LENGTH */
		/* WAIT_LOOP */
		for( loop_cnt=0 ; loop_cnt < hash_block_cnt ; loop_cnt++ )
			for( hash_result_index=0 ; hash_result_index < R_T4_DRIVER_RX_HASH_LENGTH ; hash_result_index++ )
				hash_result[lan_port_no][hash_result_index] ^= message[(R_T4_DRIVER_RX_HASH_LENGTH * loop_cnt) + hash_result_index];

	/* XOR calculation to self for fraction */
	/* WAIT_LOOP */
	for( hash_result_index=0 ; hash_result_index < hash_block_odd ; hash_result_index++ )
		hash_result[lan_port_no][hash_result_index] ^= message[(R_T4_DRIVER_RX_HASH_LENGTH * hash_block_cnt) + hash_result_index];
	/* Get random value */
	get_random_number( &random_value[0], R_T4_DRIVER_RX_HASH_LENGTH - hash_block_odd );
	/* XOR calculation by random value */
	/* WAIT_LOOP */
	for( hash_result_index=hash_block_odd ; hash_result_index < R_T4_DRIVER_RX_HASH_LENGTH ; hash_result_index++ )
		hash_result[lan_port_no][hash_result_index] ^= random_value[hash_result_index - hash_block_odd];
	*hash = &hash_result[lan_port_no][0];
	*hash_len = hash_result_len;
}

EXPORT BOOL lan_check_link(UB lan_port_no)
{
	return ETHER_SUCCESS == R_ETHER_CheckLink_ZC( lan_port_no ) ? TRUE : FALSE;
}

LOCAL ER sleep_task(ID cepid, BOOL ip_type)
{
	( ip_type == IP_TYPE_TCP ? tcp_tsk_tbl : udp_tsk_tbl )[cepid-1] = tk_get_tid( );
	return tk_slp_tsk( TMO_FEVR );
}

LOCAL ER wakeup_task(ID cepid, BOOL ip_type)
{
	return tk_wup_tsk(( ip_type == IP_TYPE_TCP ? tcp_tsk_tbl : udp_tsk_tbl )[cepid - 1] );
}

EXPORT void ena_int(void)
{
	tk_ena_dsp( );
}

EXPORT void dis_int(void)
{
	tk_dis_dsp( );
}

EXPORT void tcpudp_act_cyc(UB cycact)
{
	if( cycact == 1 )
		tk_sta_cyc( ether_objid[1] );
	else
		tk_stp_cyc( ether_objid[1] );
}

ER system_callback(UB channel, UW eventid, void *param)
{
	return E_OK;
}