/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

#include <stdio.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_rtc.h>

LOCAL DATE_TIM dt;
LOCAL VB buf[32];

EXPORT INT usermain( void )
{
ID dd;
SZ asize;

	if( rtcDrvEntry( ) < E_OK )					// RTCドライバを登録
		goto ERROR;
	if( ( dd = tk_opn_dev( RTC_DEVNM, TD_UPDATE ) ) < E_OK )	// RTCドライバをオープン
		goto ERROR;
	
	tm_putstring("Input now date and time.\n"
		     "Year:Month:Day:Week:Hour:Minute:Second\n"
		     "Week is 0 -> Sunday ... 6 -> Saturday\n"
		     "Ex. 2000:1:1:6:12:34:56\n\n");			// 現在時刻を入力
	tm_getline( buf );
	sscanf( buf, "%ld:%ld:%ld:%ld:%ld:%ld:%ld", &dt.d_year, &dt.d_month, &dt.d_day, &dt.d_wday, &dt.d_hour, &dt.d_min, &dt.d_sec );
	tm_putstring("\n");
	dt.d_year -= 1900;						// 現在時刻をRTCドライバに書き込み
	if( tk_swri_dev( dd, DN_CKDATETIME, &dt, sizeof(dt), &asize ) < E_OK || asize != sizeof(dt) )
		goto ERROR;
	while( 1 )  {
		tk_dly_tsk( 1 );
		if( tk_srea_dev( dd, DN_CKDATETIME, &dt, sizeof(dt), &asize ) < E_OK || asize != sizeof(dt) )
			goto ERROR;					// 現在時刻をRTCドライバから読み込み
		dt.d_year += 1900;
		tm_printf("\r%ld:%.2ld:%.2ld:%ld:%.2ld:%.2ld:%.2ld", dt.d_year, dt.d_month, dt.d_day, dt.d_wday, dt.d_hour, dt.d_min, dt.d_sec );
	}
ERROR:
	return 0;
}