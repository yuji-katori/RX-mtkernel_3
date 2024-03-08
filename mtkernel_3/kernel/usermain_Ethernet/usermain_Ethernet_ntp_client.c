/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */
#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "dev_ether.h"

// UTC時刻のオーバフローを管理するマクロ名です。本プログラム起動時の時刻が
// 2036年2月6日6時28分15秒を超えていない場合は 0 を指定します。
// 2036年2月6日6時28分15秒を超える場合は 1 を指定します。
// 以後、4,294,967,295秒を超える毎に +1 を指定します。
#define UTCBASE		0	// 現在は2036年2月6日6時28分15秒 未満
// 日本の標準時刻を使用するか、グリニッチ標準時刻を使用するかを指定します。
// グリニッチ標準時刻を使用する場合は以下のマクロ名をコメントアウトします。
#define JPNTIME			// 日本の標準時刻を使用
// NTPサーバのIPアドレスです。１つだけマクロ名を有効にします。
#define NTPSERVER	0x85F3EEF3	// 133.243.238.243 NICT(独立行政法人情報通信研究機構)
//#define NTPSERVER	0x850F4008	// 133. 15. 64.  8 豊橋技術科学大学

void UTCtoTS(W hi, UW lo, UW *s, UW *f);
void TStoUTC(UW s, UW f, W *hi, UW *lo);
void UTCtoDATETIM(W hi, UW lo);

#define T4_WORK_SIZE (2796)
LOCAL UW tcpudp_work[(T4_WORK_SIZE / sizeof(UW))];

typedef struct {		// NTPパケット構造
	UB	li_vn_mode, stratum, poll, precision;
	UW	rootDelay, rootDispersion, refId;
	UW	refTm_s, refTm_f,  origTm_s, origTm_f,  rxTm_s, rxTm_f,  txTm_s, txTm_f;
} NTP_PAKET;
LOCAL NTP_PAKET buf;		// 送受信バッファ(NTPパケット)

#ifdef __LIT
#define TS(x)	__revl(x)
#else
#define TS(x)	(x)
#endif

LOCAL void ntp_client_tsk(INT stacd, void *exinf)
{
LOCAL T_IPV4EP dstaddr;
LOCAL SYSTIM tim;
ID cepid = stacd;
D t1, t2, t3, t4;
	dstaddr.ipaddr = NTPSERVER;					// サーバのIPアドレスを設定
	dstaddr.portno = 123;						// サーバのポート番号を設定
	// 自局のポート番号はCFG_UDP_CEPID#_PORT_NUMBERで123を指定
	t1 = 4294967296000 * UTCBASE;					// UTCのベース時刻を設定
	tim.hi = t1 >> 32;  tim.lo = t1;				// システム時刻用の変数を初期化
	tk_set_utc( &tim );						// UTC時刻を初期化
	while( 1 )  {
		buf.li_vn_mode = 0x23;					// LI, VN, MODEを設定
		buf.poll = 17;						// Pollを設定
		buf.stratum = buf.precision = buf.rootDelay = buf.rootDispersion = buf.refId = 0;
		tk_get_utc( &tim );					// UTC時刻を参照
		UTCtoTS( tim.hi, tim.lo, &buf.txTm_s, &buf.txTm_f );	// 送信のタイムスタンプを設定
		t1 = ( ( t1 = tim.hi ) << 32 ) + tim.lo;		// t1のUTC時刻(64ビット)を設定
		if( udp_snd_dat( cepid, &dstaddr, &buf, sizeof(buf), 3000 ) != sizeof(buf) )
			break;						// サーバへデータ送信
		if( udp_rcv_dat( cepid, &dstaddr, &buf, sizeof(buf), 3000 ) != sizeof(buf) )
			break;						// サーバからデータ受信
		tk_get_utc( &tim );					// t4のUTC時刻を参照
		UTCtoTS( tim.hi, tim.lo, &buf.refTm_s, &buf.refTm_f );	// t4のタイムスタンプを設定
		t4 = ( ( t4 = tim.hi ) << 32 ) + tim.lo;		// t4のUTC時刻(64ビット)を設定
		TStoUTC( buf.rxTm_s, buf.rxTm_f, &tim.hi, &tim.lo );	// t2をUTC時刻に変換
		t2 = ( ( t2 = tim.hi ) << 32 ) + tim.lo;		// t2のUTC時刻(64ビット)を設定
		TStoUTC( buf.txTm_s, buf.txTm_f, &tim.hi, &tim.lo );	// t3をUTC時刻に変換
		t3 = ( ( t3 = tim.hi ) << 32 ) + tim.lo;		// t3のUTC時刻(64ビット)を設定
		t1 = ( ( t2 - t1 ) + ( t3 - t4 ) ) / 2;			// オフセットを算出
		tk_get_utc( &tim );					// UTC時刻を参照
		t1 += t4 = ( ( t4 = tim.hi ) << 32 ) + tim.lo;		// 同期UTC時刻を算出
		t1 += t4 = ( t4 / 1000 >> 32 << 32 ) * 1000;		// 2036年問題を補正
		tim.hi = t1 >> 32;  tim.lo = t1;			// 同期UTC時刻を設定
		tk_set_utc( &tim );					// 同期UTC時刻を設定
		UTCtoDATETIM( tim.hi, tim.lo );				// UTC時刻を表示
		buf.origTm_s = buf.txTm_s;  buf.origTm_f = buf.txTm_f;	// 次回のt3をバッファに設定
		buf.rxTm_s   = buf.refTm_s; buf.rxTm_f   = buf.refTm_f;	// 次回のt4をバッファに設定
		UTCtoTS( tim.hi, tim.lo, &buf.refTm_s, &buf.refTm_f );	// 同期UTC時刻のタイムスタンプを設定
		tk_dly_tsk( 1000 * 60 * 60 * 24 );			// 24時間待ち
	}
	tk_wup_tsk( (ID)exinf );					// 初期化タスクを起床
	tk_ext_tsk( );							// タスクの終了
}

void usermain(void)
{
ID objid;
T_CTSK t_ctsk;
	if( lan_open( ) != E_OK )				// LANソケットの動作開始
		goto Err;
	if( tcpudp_get_ramsize( ) > sizeof( tcpudp_work ) )	// ワークエリアのサイズを確認
		goto Err;
	if( tcpudp_open( tcpudp_work ) != E_OK )		// TCP/UDP通信を開始
		goto Err;
	t_ctsk.exinf = (void*)tk_get_tid( );			// 自タスクのID番号を拡張情報に設定
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// タスクの属性を設定
	t_ctsk.stksz = 356;					// タスクのスタックサイズを設定
	t_ctsk.itskpri = 10;					// タスクの優先度を設定（任意）
	t_ctsk.task =  ntp_client_tsk;				// タスクの起動番地を設定
	strcpy( t_ctsk.dsname, "ntp_cli" );			// タスクのデバッガサポート名を設定
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// クライアントタスクの生成
		goto Err;
	if( tk_sta_tsk( objid, 1 ) < E_OK )			// クライアントタスクの起動
		goto Err;					// 通信端点IDはタスクの起動コードに設定
	tk_slp_tsk( TMO_FEVR );					// 通信の終了を待つ
	tcpudp_close( );					// TCP/UDP通信を終了
	lan_close( );						// LANソケットの動作停止
Err:
	while( 1 )  ;
}


void UTCtoTS(W hi, UW lo, UW *s, UW *f)
{
D ll_tim;
	ll_tim = ( ( ll_tim = hi ) << 32 ) + lo;		// UTC時刻を64ビットに変換
	*s = TS( ll_tim / 1000 );				// タイムスタンプの秒数を設定
	*f = TS( ( ll_tim % 1000 << 32 ) / 1000 );		// タイムスタンプの端数を設定
}

void TStoUTC(UW s, UW f, W *hi, UW *lo)
{
D ll_tim;
	ll_tim  = TS( s ) * 1000LL + ( TS( f ) * 1000LL >> 32 );// タイムスタンプを64ビットに変換
	*hi = ll_tim >> 32;					// UTC時刻の上位32ビットを設定
	*lo = ll_tim;						// UTC時刻の下位32ビットを設定
}

void UTCtoDATETIM(W hi, UW lo)
{
UW  l_tim;
D  ll_tim, x;
INT y, m, d, h, n, s, z;
	ll_tim = ( ( ll_tim = hi ) << 32 ) + lo;		// UTC時刻を64ビットに変換
#ifdef JPNTIME							// 日本の標準時刻か？
	ll_tim += 32400000;					// 時差９時間分を加算
#endif
	if( ll_tim < 3155673600000 )				// 2000年未満のカウント数か？
		y = 1900;					// 開始は1900年
	else  {
		ll_tim -= 3155673600000;			// 100年分のカウント数を減算
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
	tm_printf("\r%ld:%.2ld:%.2ld:%.2ld:%.2ld:%.2ld\n", y, m, d, h, n, s);
}