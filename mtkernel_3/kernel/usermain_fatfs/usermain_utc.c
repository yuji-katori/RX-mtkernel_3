/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

#include <stdio.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

LOCAL INT y, m, d, h, n, s;
LOCAL VB buf[32];
LOCAL SYSTIM tim;

void DATETIMtoTRON(void)
{
D  ll_tim, x;
INT i, z;
	ll_tim = 1000LL * ( s + 60 * ( n + 60L * ( h + ( d - 1 ) * 24 ) ) );
	if( y >= 2000 )  {				// 2000年以降か？
		ll_tim += 946684800000;			// 30年分のカウント値を加算
		i = 2000;				// 2000年を開始年とする
	}
	else
		i = 1970;				// 1970念を開始年とする
	for(  ;  ; i++ )  {
		if( i % 4 )				// 4で割り切れないか？
			// 11 --> 31day, 10 --> 30day, 01 --> 29day, 00 --> 28day
			// Dec Nov Oct Sep Aug Jul Jun May Apr Mar Feb Jan (Dec not used)
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// うるう年ではない
		else if( i % 100 )			// 100で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// うるう年
		else if( i % 400 )			// 400で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// うるう年ではない
		else					// 400で割り切る
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// うるう年
		if( i == y )				// 今年か？
			break;
		ll_tim += x;				// １年分の秒数を加算
	}
	for( i=1 ; i!=m ; i++, z>>=2 )  {
		if( z & 2 )				// 31日か30日か？
			if( z & 1 )			// 31日か？
				x = 2678400000;		// 31日
			else
				x = 2592000000;		// 30日
		else
			if( z & 1 )			// 29日か28日か？
				x = 2505600000;		// 29日
			else
				x = 2419200000;		// 28日
		ll_tim += x;				// １ケ月分の秒数を加算
	}
	tim.hi = ll_tim >> 32;				// 上位32ビットを設定
	tim.lo = ll_tim;				// 下位32ビットを設定
}

void TRONtoDATETIM(void)
{
UW  l_tim;
D  ll_tim, x;
INT z;
	ll_tim = ( ( ll_tim = tim.hi ) << 32 ) + tim.lo;// UTC時刻を64ビットに変換
	if( ll_tim < 946684800000 )			// 2000年以下のカウント数か？
		y = 1970;				// 開始は1970年
	else  {
		ll_tim -= 946684800000;			// 30年分のカウント数を減算
		for( y=2000 ;  ; y+=400 )  {		// 開始は2000年
			if( ll_tim < 12622780800000 )	// 400年分の秒数以下か？
				break;
			ll_tim -= 12622780800000;	// 400年分のカウント数を減算
		}
		if( ll_tim >= 3155760000000 )  {	// 100年以上のカウント数か？
			ll_tim -= 3155760000000;	// 100年分のカウント数を減算
			for(  ;  ; y+=100 )  {
				if( ll_tim < 3155673600000 )	// 100年未満のカウント数か？
					break;
				ll_tim -= 3155673600000;	// 100年分のカウント数を減算
			}
		}
	}
	for(  ;  ; y++ )  {
		if( y % 4 )				// 4で割り切れないか？
			// 11 --> 31day, 10 --> 30day, 01 --> 29day, 00 --> 28day
			// Dec Nov Oct Sep Aug Jul Jun May Apr Mar Feb Jan (Dec not used)
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// うるう年ではない
		else if( y % 100 )			// 100で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// うるう年
		else if( y % 400 )			// 400で割り切れないか？
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// うるう年ではない
		else					// 400で割り切る
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// うるう年
		if( ll_tim < x )			// 残りの秒数は１年以内か？
			break;
		ll_tim -= x;				// １年分の秒数を減算
	}
	for( m=1 ;  ; m++, z>>=2 )  {
		if( z & 2 )				// 31日か30日か？
			if( z & 1 )			// 31日か？
				x = 2678400000;		// 31日
			else
				x = 2592000000;		// 30日
		else
			if( z & 1 )			// 29日か28日か？
				x = 2505600000;		// 29日
			else
				x = 2419200000;		// 28日
		if( ll_tim < x )			// 残りの秒数は１ケ月以内か？
			break;
		ll_tim -= x;				// １ケ月分の秒数を減算
	}
	l_tim = ll_tim;					// long long型からlong型に変換
	d = l_tim / 86400000 + 1, l_tim %= 86400000;	// 日数を算出
	h = l_tim / 3600000     , l_tim %= 3600000;	// 時数を算出
	n = l_tim / 60000       , l_tim %= 60000;	// 分数を算出
	s = l_tim / 1000;				// 秒数を算出
}

EXPORT INT usermain( void )
{
D ll_tim;
	tm_putstring("Input now date and time.\n"
		     "Year:Month:Day:Hour:Minute:Second\n"
		     "Ex. 2000:1:1:12:34:56\n\n");	// 現在時刻を入力
	tm_getline( buf );
	sscanf( buf, "%ld:%ld:%ld:%ld:%ld:%ld", &y, &m, &d, &h, &n, &s );
	tm_putstring("\n");

	DATETIMtoTRON( );				// 年月日時分秒をUTC時刻に変換
	tk_set_utc( &tim );				// UTC時刻を設定
	while( 1 )  {
		tk_dly_tsk( 333 );			// 333msの待ち
		tk_get_utc( &tim );			// UTC時刻を参照
		TRONtoDATETIM( );			// UTC時刻を年月日時分秒に変換
		tm_printf("\r%ld:%.2ld:%.2ld:%.2ld:%.2ld:%.2ld", y, m, d, h, n, s );
		ll_tim = ( ( ll_tim = tim.hi ) << 32 ) + tim.lo;	// UTC時刻を64ビットに変換
		ll_tim /= 1000;
	}
}