/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	fatlower.c
 */

#include <tk/tkernel.h>
#include <dev_rtc.h>
#include "ff.h"

#if _FS_READONLY == 0

DWORD get_fattime (void)
{

#if FF_FS_NORTC == 1
ID rtc;
ER ercd;
DATE_TIM time;
SZ asize;

#ifdef CLANGSPEC
	rtc = tk_opn_dev( RTC_DEVNM, TD_READ );
#else
	rtc = tk_opn_dev( (UB*)RTC_DEVNM, TD_READ );
#endif	/* CLANGSPEC */
	if( rtc >= E_OK )  {
		ercd = tk_srea_dev( rtc, DN_CKDATETIME, &time, sizeof(DATE_TIM), &asize );
		tk_cls_dev( rtc, 0 );
		if( ercd >= E_OK )  {
			return    ((DWORD)(time.d_year - 80) << 25)
				| ((DWORD) time.d_month      << 21)
				| ((DWORD) time.d_day 	     << 16)
				| ((DWORD) time.d_hour       << 11)
				| ((DWORD) time.d_min        <<  5)
				| ((DWORD) time.d_sec        >>  1);
		}
	}
	return	  ((DWORD) 20 << 25)	// 2000/1/1 0:0:0
		| ((DWORD)  1 << 21)
		| ((DWORD)  1 << 16)
		| ((DWORD)  0 << 11)
		| ((DWORD)  0 <<  5)
		| ((DWORD)  0 >>  1);
#elif FF_FS_NORTC == 2
SYSTIM tim;
UW  l_tim;
D  ll_tim, x;
INT z, y, m, d, h, n, s;
	tk_get_utc( &tim );					// UTC時刻を参照
	ll_tim = ( ( ll_tim = tim.hi ) << 32 ) + tim.lo;	// UTC時刻を64ビットに変換
	if( ll_tim < 946684800000 )				// 2000年以下のカウント数か？
		y = 1970;					// 開始は1970年
	else  {
		ll_tim -= 946684800000;				// 30年分のカウント数を減算
		for( y=2000 ;  ; y+=400 )  {			// 開始は2000年
			if( ll_tim < 12622780800000 )		// 400年分の秒数以下か？
				break;
			ll_tim -= 12622780800000;		// 400年分のカウント数を減算
		}
		if( ll_tim >= 3155760000000 )  {		// 100年以上のカウント数か？
			ll_tim -= 3155760000000;		// 100年分のカウント数を減算
			for(  ;  ; y+=100 )  {
				if( ll_tim < 3155673600000 )	// 100年未満のカウント数か？
					break;
				ll_tim -= 3155673600000;	// 100年分のカウント数を減算
			}
		}
	}
	for(  ;  ; y++ )  {
		if( y % 4 )					// 4で割り切れないか？
			// 11 --> 31day, 10 --> 30day, 01 --> 29day, 00 --> 28day
			// Dec Nov Oct Sep Aug Jul Jun May Apr Mar Feb Jan (Dec not used)
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;		// うるう年ではない
		else if( y % 100 )				// 100で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;		// うるう年
		else if( y % 400 )				// 400で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;		// うるう年ではない
		else						// 400で割り切る
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;		// うるう年
		if( ll_tim < x )				// 残りの秒数は１年以内か？
			break;
		ll_tim -= x;					// １年分の秒数を減算
	}
	for( m=1 ;  ; m++, z>>=2 )  {
		if( z & 2 )					// 31日か30日か？
			if( z & 1 )				// 31日か？
				x = 2678400000;			// 31日
			else
				x = 2592000000;			// 30日
		else
			if( z & 1 )				// 29日か28日か？
				x = 2505600000;			// 29日
			else
				x = 2419200000;			// 28日
		if( ll_tim < x )				// 残りの秒数は１ケ月以内か？
			break;
		ll_tim -= x;					// １ケ月分の秒数を減算
	}
	l_tim = ll_tim;						// long long型からlong型に変換
	d = l_tim / 86400000 + 1, l_tim %= 86400000;		// 日数を算出
	h = l_tim / 3600000     , l_tim %= 3600000;		// 時数を算出
	n = l_tim / 60000       , l_tim %= 60000;		// 分数を算出
	s = l_tim / 1000;					// 秒数を算出
	return    ((DWORD)(y - 1980) << 25)
		| ((DWORD) m         << 21)
		| ((DWORD) d 	     << 16)
		| ((DWORD) h         << 11)
		| ((DWORD) n         <<  5)
		| ((DWORD) s         >>  1);
#endif
}
#endif	/* _FS_READONLY */