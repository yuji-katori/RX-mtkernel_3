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
	t_dint.inthdr = cmt2_hdr;				// timer_hdr�̋N���A�h���X
	if( tk_def_int( VECT( CMT2, CMI2 ), &t_dint ) != E_OK )	// CMT2�����݃n���h���̒�`
		goto ERROR;

	PORTD.PDR.BIT.B6 = 1;					// LED0��L��
	SYSTEM.PRCR.WORD = 0xA502;				// �v���e�N�g����
	MSTP( CMT2 ) = 0;					// CMT2��CMT3�̃��W���[���X�^���o�C������
	SYSTEM.PRCR.WORD = 0xA500;				// �v���e�N�g�ݒ�
	CMT2.CMCOR = 27000000/10/512-1;				// 100ms
	CMT2.CMCR.WORD = 0x0043;				// �R���y�A�}�b�`�����݋���,��/512
	EnableInt( VECT( CMT2, CMI2 ), 9 );			// �����݃��x��9
	CMT.CMSTR1.BIT.STR2 = 1;				// �^�C�}�X�^�[�g
	if( vecttbl[VECT( CMT3, CMI3 )] == cmt3_cmi3 )  {
		CMT3.CMCOR = 27000000/5/512-1;			// 200ms
		CMT3.CMCR.WORD = 0x0043;			// �R���y�A�}�b�`�����݋���,��/512
		EnableInt( VECT( CMT3, CMI3 ), 14 );		// �����݃��x��14(OS�Ǘ��O�����݃��x��)
		CMT.CMSTR1.BIT.STR3 = 1;			// �^�C�}�X�^�[�g
	}

	while( 1 )  tk_slp_tsk(TMO_FEVR);			// �N���҂�
ERROR:
	return 0;
}

EXPORT void tsk_a(INT stacd, void *exinf)
{
	while( 1 )  {
		tk_slp_tsk( 500 );
		PORTD.PODR.BIT.B6 ^= 1;				// LED0�̓_��
	}
}

EXPORT void cmt2_hdr(UINT intno)
{
	tk_wup_tsk( ObjID[TSK_A] );				// tsk_a���N��
}


// OS�Ǘ��O�����݃n���h���̋L�q��
// vector.src�̃x�N�^�ԍ�31���R�����g�A�E�g
// �ȉ���#pragma���̃R�����g�A�E�g������
//#pragma interrupt cmt3_cmi3(vect=VECT( CMT3, CMI3 ),enable)
EXPORT void cmt3_cmi3(void)
{
static unsigned char cnt;
	if( ++ cnt == 25 )  {					// 5�b���H
		cnt = 0;					// �J�E���g�l�̃N���A
		CMT.CMSTR1.BIT.STR2 ^= 1;			// CMT2�̏�Ԃ𔽓]
	}
}
