/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
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
EXPORT void cmt2_hdr(UINT intno);
EXPORT void cmt3_cmi3(void);

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
	t_dint.inthdr = cmt2_hdr;				// timer_hdrの起動アドレス
	if( tk_def_int( VECT( CMT2, CMI2 ), &t_dint ) != E_OK )	// CMT2割込みハンドラの定義
		goto ERROR;

	PORTD.PDR.BIT.B6 = 1;					// LED0を有効
	SYSTEM.PRCR.WORD = 0xA502;				// プロテクト解除
	MSTP( CMT2 ) = 0;					// CMT2とCMT3のモジュールスタンバイを解除
	SYSTEM.PRCR.WORD = 0xA500;				// プロテクト設定
	CMT2.CMCOR = 27000000/10/512-1;				// 100ms
	CMT2.CMCR.WORD = 0x0043;				// コンペアマッチ割込み許可,φ/512
	EnableInt( VECT( CMT2, CMI2 ), 9 );			// 割込みレベル9
	CMT.CMSTR1.BIT.STR2 = 1;				// タイマスタート
	if( vecttbl[VECT( CMT3, CMI3 )] == cmt3_cmi3 )  {
		CMT3.CMCOR = 27000000/5/512-1;			// 200ms
		CMT3.CMCR.WORD = 0x0043;			// コンペアマッチ割込み許可,φ/512
		EnableInt( VECT( CMT3, CMI3 ), 14 );		// 割込みレベル14(OS管理外割込みレベル)
		CMT.CMSTR1.BIT.STR3 = 1;			// タイマスタート
	}

	while( 1 )  tk_slp_tsk(TMO_FEVR);			// 起床待ち
ERROR:
	return 0;
}

EXPORT void tsk_a(INT stacd, void *exinf)
{
	while( 1 )  {
		tk_slp_tsk( 500 );
		PORTD.PODR.BIT.B6 ^= 1;				// LED0の点滅
	}
}

EXPORT void cmt2_hdr(UINT intno)
{
	tk_wup_tsk( ObjID[TSK_A] );				// tsk_aを起床
}


// OS管理外割込みハンドラの記述例
// vector.srcのベクタ番号31をコメントアウト
// 以下の#pragma文のコメントアウトを解除
//#pragma interrupt cmt3_cmi3(vect=VECT( CMT3, CMI3 ),enable)
EXPORT void cmt3_cmi3(void)
{
static unsigned char cnt;
	if( ++ cnt == 25 )  {					// 5秒か？
		cnt = 0;					// カウント値のクリア
		CMT.CMSTR1.BIT.STR2 ^= 1;			// CMT2の状態を反転
	}
}
