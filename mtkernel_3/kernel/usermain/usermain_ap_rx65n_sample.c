/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2022/10/31.
 *----------------------------------------------------------------------
 */

/*
 *	usermain.c (usermain)
 *	User Main
 */

#include <tk/tkernel.h>
#include <string.h>
#include "iodefine.h"

EXPORT void tsk_a(INT stacd, void *exinf);
EXPORT void cmwi0_hdr(UINT intno);
EXPORT void cmtw1_cmwi1(void);

typedef enum { TSK_A, OBJ_KIND_NUM } OBJ_KIND;
EXPORT ID ObjID[OBJ_KIND_NUM];					// IDテーブル

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
T_DINT t_dint;
ID objid;
void (**vecttbl)(void) = __sectop("C$VECT");

	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// タスク属性を設定
	t_ctsk.stksz = 1024;					// スタックサイズは1024バイト
	t_ctsk.itskpri = 2;					// tsk_aの優先度
#ifdef CLANGSPEC
	t_ctsk.task =  tsk_a;					// tsk_aの起動アドレス
	strcpy( t_ctsk.dsname, "tsk_a" );			// tsk_aの名称
#else
	t_ctsk.task =  (FP)tsk_a;				// tsk_aの起動アドレス
	strcpy( (char*)t_ctsk.dsname, "tsk_a" );		// tsk_aの名称
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// tsk_aの生成
		goto ERROR;
	ObjID[TSK_A] = objid;
	if( tk_sta_tsk( ObjID[TSK_A], 0 ) != E_OK )		// tsk_aの起動
		goto ERROR;

	t_dint.intatr = TA_HLNG;				// 割込みハンドラ属性
	t_dint.inthdr = cmwi0_hdr;				// timer_hdrの起動アドレス
	if( tk_def_int( VECT( CMTW0, CMWI0 ), &t_dint ) != E_OK )	// CMTW0割込みハンドラの定義
		goto ERROR;

	PORTC.PDR.BIT.B0 = 1;					// LD2を有効
	SYSTEM.PRCR.WORD = 0xA502;				// プロテクト解除
	MSTP( CMTW0 ) = 0;					// CMTW0のモジュールスタンバイを解除
	MSTP( CMTW1 ) = 0;					// CMTW1のモジュールスタンバイを解除
	SYSTEM.PRCR.WORD = 0xA500;				// プロテクト設定
	CMTW0.CMWCOR = 60000000/10/8-1;				// 100ms
	CMTW0.CMWCR.BIT.CMWIE = 1;				// コンペアマッチ割込み許可,φ/8
	CMTW0.CMWIOR.BIT.CMWE = 1;				// コンペアマッチ許可
	EnableInt( VECT( CMTW0, CMWI0 ), 9 );			// 割込みレベル9
	CMTW0.CMWSTR.BIT.STR = 1;				// タイマスタート
	if( vecttbl[VECT( CMTW1, CMWI1 )] == cmtw1_cmwi1 )  {
		CMTW1.CMWCOR = 60000000/5/8-1;			// 200ms
		CMTW1.CMWCR.BIT.CMWIE = 1;			// コンペアマッチ割込み許可,φ/8
		CMTW1.CMWIOR.BIT.CMWE = 1;			// コンペアマッチ許可
		EnableInt( VECT( CMTW1, CMWI1 ), 14 );		// 割込みレベル14(OS管理外割込みレベル)
		CMTW1.CMWSTR.BIT.STR = 1;			// タイマスタート
	}

	while( 1 )  tk_slp_tsk(TMO_FEVR);			// 起床待ち
ERROR:
	return 0;
}

EXPORT void tsk_a(INT stacd, void *exinf)
{
	while( 1 )  {
		tk_slp_tsk( 500 );
		PORTC.PODR.BIT.B0 ^= 1;				// LD2の点滅
	}
}

EXPORT void cmwi0_hdr(UINT intno)
{
	tk_wup_tsk( ObjID[TSK_A] );				// tsk_aを起床
}


// OS管理外割込みハンドラの記述例
// vector.srcのベクタ番号31をコメントアウト
// 以下の#pragma文のコメントアウトを解除
//#pragma interrupt cmtw1_cmwi1(vect=VECT( CMTW1, CMWI1 ),enable)
EXPORT void cmtw1_cmwi1(void)
{
static unsigned char cnt;
	if( ++ cnt == 25 )  {					// 5秒か？
		cnt = 0;					// カウント値のクリア
		CMTW0.CMWSTR.BIT.STR ^= 1;			// CMTW0の状態を反転
	}
}
