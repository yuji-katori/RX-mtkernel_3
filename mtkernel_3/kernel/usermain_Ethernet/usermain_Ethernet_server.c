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

#define T4_WORK_SIZE (5112)
LOCAL UW tcpudp_work[T4_WORK_SIZE/sizeof(UW) + 1];

LOCAL char buf[3][1460];		// 受信バッファ（通信端点数分確保）

LOCAL void server_tsk(INT stacd, void *exinf)
{
T_IPV4EP dstaddr;
ID cepid = stacd+1, repid = (ID)exinf;
INT len;  ER ercd;
	while( 1 )  {
		tcp_acp_cep( cepid, repid, &dstaddr, TMO_FEVR );
		// 接続要求待ち、通信端点IDはタスクの起動コードから入手、受け口ポートIDはタスクの拡張情報から入手
		while( 1 )  {
			// パケットが分割されない時の受信手順。分割されないなら一度の受信要求で完了
//			if( (len = tcp_rcv_dat( cepid, buf[stacd], 1460, TMO_FEVR )) <= E_OK )
//				break;
			// パケットが分割される可能性がある時の受信手順。以下の例は300msのタイムアウトで判断
			// 最初のパケットは永久待ちで受信要求を行い、２回目以降は300msのタイムアウトを設定
			len = tcp_rcv_dat( cepid, buf[stacd], 1460, TMO_FEVR );
			while( (ercd=tcp_rcv_dat( cepid, &buf[stacd][len], 1460, 30 )) > E_OK )
				len += ercd;
			if( ercd != E_TMOUT )					// エラーコードがタイムアウトでなければ通信終了
				break;						// 無限ループを抜け出す
			buf[stacd][len] = '\0';					// 受信バッファにヌルコードを設定
			tm_printf("cepid = %d : %s\n", cepid, buf[stacd] );	// ターミナルに表示
			tcp_snd_dat( cepid, buf[stacd], len, TMO_FEVR );	// クライアントにエコーバック
		}
		tcp_sht_cep( cepid );						// 切断準備
		tcp_cls_cep( cepid, TMO_FEVR );					// 通信を切断
	}
}

void usermain(void)
{
int i;
ID objid;
T_CTSK t_ctsk;
	if( lan_open( ) != E_OK )				// LANソケットの動作開始
		goto Err;
	if( tcpudp_get_ramsize( ) > sizeof( tcpudp_work ) )	// ワークエリアのサイズを確認
		goto Err;
	if( tcpudp_open( tcpudp_work ) != E_OK )		// TCP/UDP通信を開始
		goto Err;
	t_ctsk.exinf = (void*)1;				// 受け口ポートIDを拡張情報に設定
	// 受け口ポートID(1)の詳細は config_tcpudp.h の CFG_TCP_REPID1_PORT_NUMBER で定義
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// タスクの属性を設定
	t_ctsk.stksz = 512;					// タスクのスタックサイズを設定
	t_ctsk.itskpri = 10;					// タスクの優先度を設定（任意）
	t_ctsk.task =  server_tsk;				// タスクの起動番地を設定
	strcpy( t_ctsk.dsname, "server" );			// タスクのデバッガサポート名を設定
	for( i=0 ; i<3 ; i++ )  {				// 用意した通信端点数分のループ
		// 用意した通信端点の個数は config_tcpudp.h の CFG_TCP_CEPID_NUM で定義
		if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )	// サーバータスクの生成
			goto Err;
		if( tk_sta_tsk( objid, i ) < E_OK )		// サーバータスクの起動
			goto Err;				// 通信端点IDはタスクの起動コードに設定
	}
	tk_slp_tsk( TMO_FEVR );					// 起床待ちとなり、サーバータスクに制御を渡す
	tcpudp_close( );					// TCP/UDP通信を終了
	lan_close( );						// LANソケットの動作停止
Err:
	while( 1 )  ;
}
