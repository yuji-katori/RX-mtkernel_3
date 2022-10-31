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
EXPORT ID ObjID[OBJ_KIND_NUM];					// ID�e�[�u��

EXPORT INT usermain( void )
{
T_CTSK t_ctsk;
T_DINT t_dint;
ID objid;
void (**vecttbl)(void) = __sectop("C$VECT");

	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// �^�X�N������ݒ�
	t_ctsk.stksz = 1024;					// �X�^�b�N�T�C�Y��1024�o�C�g
	t_ctsk.itskpri = 2;					// tsk_a�̗D��x
#ifdef CLANGSPEC
	t_ctsk.task =  tsk_a;					// tsk_a�̋N���A�h���X
	strcpy( t_ctsk.dsname, "tsk_a" );			// tsk_a�̖���
#else
	t_ctsk.task =  (FP)tsk_a;				// tsk_a�̋N���A�h���X
	strcpy( (char*)t_ctsk.dsname, "tsk_a" );		// tsk_a�̖���
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// tsk_a�̐���
		goto ERROR;
	ObjID[TSK_A] = objid;
	if( tk_sta_tsk( ObjID[TSK_A], 0 ) != E_OK )		// tsk_a�̋N��
		goto ERROR;

	t_dint.intatr = TA_HLNG;				// �����݃n���h������
	t_dint.inthdr = cmwi0_hdr;				// timer_hdr�̋N���A�h���X
	if( tk_def_int( VECT( CMTW0, CMWI0 ), &t_dint ) != E_OK )	// CMTW0�����݃n���h���̒�`
		goto ERROR;

	PORTC.PDR.BIT.B0 = 1;					// LD2��L��
	SYSTEM.PRCR.WORD = 0xA502;				// �v���e�N�g����
	MSTP( CMTW0 ) = 0;					// CMTW0�̃��W���[���X�^���o�C������
	MSTP( CMTW1 ) = 0;					// CMTW1�̃��W���[���X�^���o�C������
	SYSTEM.PRCR.WORD = 0xA500;				// �v���e�N�g�ݒ�
	CMTW0.CMWCOR = 60000000/10/8-1;				// 100ms
	CMTW0.CMWCR.BIT.CMWIE = 1;				// �R���y�A�}�b�`�����݋���,��/8
	CMTW0.CMWIOR.BIT.CMWE = 1;				// �R���y�A�}�b�`����
	EnableInt( VECT( CMTW0, CMWI0 ), 9 );			// �����݃��x��9
	CMTW0.CMWSTR.BIT.STR = 1;				// �^�C�}�X�^�[�g
	if( vecttbl[VECT( CMTW1, CMWI1 )] == cmtw1_cmwi1 )  {
		CMTW1.CMWCOR = 60000000/5/8-1;			// 200ms
		CMTW1.CMWCR.BIT.CMWIE = 1;			// �R���y�A�}�b�`�����݋���,��/8
		CMTW1.CMWIOR.BIT.CMWE = 1;			// �R���y�A�}�b�`����
		EnableInt( VECT( CMTW1, CMWI1 ), 14 );		// �����݃��x��14(OS�Ǘ��O�����݃��x��)
		CMTW1.CMWSTR.BIT.STR = 1;			// �^�C�}�X�^�[�g
	}

	while( 1 )  tk_slp_tsk(TMO_FEVR);			// �N���҂�
ERROR:
	return 0;
}

EXPORT void tsk_a(INT stacd, void *exinf)
{
	while( 1 )  {
		tk_slp_tsk( 500 );
		PORTC.PODR.BIT.B0 ^= 1;				// LD2�̓_��
	}
}

EXPORT void cmwi0_hdr(UINT intno)
{
	tk_wup_tsk( ObjID[TSK_A] );				// tsk_a���N��
}


// OS�Ǘ��O�����݃n���h���̋L�q��
// vector.src�̃x�N�^�ԍ�31���R�����g�A�E�g
// �ȉ���#pragma���̃R�����g�A�E�g������
//#pragma interrupt cmtw1_cmwi1(vect=VECT( CMTW1, CMWI1 ),enable)
EXPORT void cmtw1_cmwi1(void)
{
static unsigned char cnt;
	if( ++ cnt == 25 )  {					// 5�b���H
		cnt = 0;					// �J�E���g�l�̃N���A
		CMTW0.CMWSTR.BIT.STR ^= 1;			// CMTW0�̏�Ԃ𔽓]
	}
}
