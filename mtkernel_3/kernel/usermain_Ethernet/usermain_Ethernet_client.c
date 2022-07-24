/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */
#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "dev_ether.h"

#define T4_WORK_SIZE (2796)
LOCAL UW tcpudp_work[(T4_WORK_SIZE / sizeof(UW)) + 1];

LOCAL char sbuf[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";	// 送信バッファ
LOCAL char rbuf[1460];					// 受信バッファ

LOCAL void client_tsk(INT stacd, void *exinf)
{
LOCAL T_IPV4EP dstaddr, myaddr;
ID cepid = stacd;  ER ercd;
INT i, len;
	dstaddr.ipaddr = (192U<<24) + (168<<16) + (1<<8) + 200;		// サーバーのIPアドレスを設定
	dstaddr.portno = 1024;						// サーバーのポート番号を設定
	if( tcp_con_cep( cepid, &myaddr, &dstaddr, 1000 ) != E_OK )	// 接続要求の発行
		goto Err;
	for( i=0 ; i<10 ; i++ )  {
		tcp_snd_dat( cepid, sbuf, sizeof(sbuf)-1-i, TMO_FEVR );	// サーバーにデータ送信
		// パケットが分割されない時の受信手順。分割されないなら一度の受信要求で完了
//		len = tcp_rcv_dat( cepid, rbuf, 1460, TMO_FEVR );
		// パケットが分割される可能性がある時の受信手順。以下の例は300msのタイムアウトで判断
		// 最初のパケットは永久待ちで受信要求を行い、２回目以降は300msのタイムアウトを設定
		len = tcp_rcv_dat( cepid, rbuf, 1460, TMO_FEVR );
		while( (ercd=tcp_rcv_dat( cepid, &rbuf[len], 1460, 30 )) > E_OK )
			len += ercd;
		rbuf[len] = '\0';					// 受信データにヌルコードを設定
		tm_printf("%s\n", rbuf );				// ターミナルに表示
		tk_dly_tsk( 5000 );					// 5秒待ち
	}
	tcp_sht_cep( cepid );						// 切断準備
	tcp_cls_cep( cepid, TMO_FEVR );					// 通信切断
Err:
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
	t_ctsk.stksz = 512;					// タスクのスタックサイズを設定
	t_ctsk.itskpri = 10;					// タスクの優先度を設定（任意）
	t_ctsk.task =  client_tsk;				// タスクの起動番地を設定
	strcpy( t_ctsk.dsname, "client" );			// タスクのデバッガサポート名を設定
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// クライアントタスクの生成
		goto Err;
	if( tk_sta_tsk( objid, 1 ) < E_OK )			// クライアントタスクの起動
		goto Err;					// 通信端点IDはタスクの起動コードに設定
	// 通信端点ID(1)の詳細は config_tcpudp.h の CFG_TCP_CEPID1_* で定義
	tk_slp_tsk( TMO_FEVR );					// 通信の終了を待つ
	tcpudp_close( );					// TCP/UDP通信を終了
	lan_close( );						// LANソケットの動作停止
Err:
	while( 1 )  ;
}