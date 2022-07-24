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

LOCAL char sbuf[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";	// ���M�o�b�t�@
LOCAL char rbuf[1460];					// ��M�o�b�t�@

LOCAL void client_tsk(INT stacd, void *exinf)
{
LOCAL T_IPV4EP dstaddr, myaddr;
ID cepid = stacd;  ER ercd;
INT i, len;
	dstaddr.ipaddr = (192U<<24) + (168<<16) + (1<<8) + 200;		// �T�[�o�[��IP�A�h���X��ݒ�
	dstaddr.portno = 1024;						// �T�[�o�[�̃|�[�g�ԍ���ݒ�
	if( tcp_con_cep( cepid, &myaddr, &dstaddr, 1000 ) != E_OK )	// �ڑ��v���̔��s
		goto Err;
	for( i=0 ; i<10 ; i++ )  {
		tcp_snd_dat( cepid, sbuf, sizeof(sbuf)-1-i, TMO_FEVR );	// �T�[�o�[�Ƀf�[�^���M
		// �p�P�b�g����������Ȃ����̎�M�菇�B��������Ȃ��Ȃ��x�̎�M�v���Ŋ���
//		len = tcp_rcv_dat( cepid, rbuf, 1460, TMO_FEVR );
		// �p�P�b�g�����������\�������鎞�̎�M�菇�B�ȉ��̗��300ms�̃^�C���A�E�g�Ŕ��f
		// �ŏ��̃p�P�b�g�͉i�v�҂��Ŏ�M�v�����s���A�Q��ڈȍ~��300ms�̃^�C���A�E�g��ݒ�
		len = tcp_rcv_dat( cepid, rbuf, 1460, TMO_FEVR );
		while( (ercd=tcp_rcv_dat( cepid, &rbuf[len], 1460, 30 )) > E_OK )
			len += ercd;
		rbuf[len] = '\0';					// ��M�f�[�^�Ƀk���R�[�h��ݒ�
		tm_printf("%s\n", rbuf );				// �^�[�~�i���ɕ\��
		tk_dly_tsk( 5000 );					// 5�b�҂�
	}
	tcp_sht_cep( cepid );						// �ؒf����
	tcp_cls_cep( cepid, TMO_FEVR );					// �ʐM�ؒf
Err:
	tk_wup_tsk( (ID)exinf );					// �������^�X�N���N��
	tk_ext_tsk( );							// �^�X�N�̏I��
}

void usermain(void)
{
ID objid;
T_CTSK t_ctsk;
	if( lan_open( ) != E_OK )				// LAN�\�P�b�g�̓���J�n
		goto Err;
	if( tcpudp_get_ramsize( ) > sizeof( tcpudp_work ) )	// ���[�N�G���A�̃T�C�Y���m�F
		goto Err;
	if( tcpudp_open( tcpudp_work ) != E_OK )		// TCP/UDP�ʐM���J�n
		goto Err;
	t_ctsk.exinf = (void*)tk_get_tid( );			// ���^�X�N��ID�ԍ����g�����ɐݒ�
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// �^�X�N�̑�����ݒ�
	t_ctsk.stksz = 512;					// �^�X�N�̃X�^�b�N�T�C�Y��ݒ�
	t_ctsk.itskpri = 10;					// �^�X�N�̗D��x��ݒ�i�C�Ӂj
	t_ctsk.task =  client_tsk;				// �^�X�N�̋N���Ԓn��ݒ�
	strcpy( t_ctsk.dsname, "client" );			// �^�X�N�̃f�o�b�K�T�|�[�g����ݒ�
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// �N���C�A���g�^�X�N�̐���
		goto Err;
	if( tk_sta_tsk( objid, 1 ) < E_OK )			// �N���C�A���g�^�X�N�̋N��
		goto Err;					// �ʐM�[�_ID�̓^�X�N�̋N���R�[�h�ɐݒ�
	// �ʐM�[�_ID(1)�̏ڍׂ� config_tcpudp.h �� CFG_TCP_CEPID1_* �Œ�`
	tk_slp_tsk( TMO_FEVR );					// �ʐM�̏I����҂�
	tcpudp_close( );					// TCP/UDP�ʐM���I��
	lan_close( );						// LAN�\�P�b�g�̓����~
Err:
	while( 1 )  ;
}