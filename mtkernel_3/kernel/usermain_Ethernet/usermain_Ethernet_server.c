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

LOCAL char buf[3][1460];		// ��M�o�b�t�@�i�ʐM�[�_�����m�ہj

LOCAL void server_tsk(INT stacd, void *exinf)
{
T_IPV4EP dstaddr;
ID cepid = stacd+1, repid = (ID)exinf;
INT len;  ER ercd;
	while( 1 )  {
		tcp_acp_cep( cepid, repid, &dstaddr, TMO_FEVR );
		// �ڑ��v���҂��A�ʐM�[�_ID�̓^�X�N�̋N���R�[�h�������A�󂯌��|�[�gID�̓^�X�N�̊g����񂩂����
		while( 1 )  {
			// �p�P�b�g����������Ȃ����̎�M�菇�B��������Ȃ��Ȃ��x�̎�M�v���Ŋ���
//			if( (len = tcp_rcv_dat( cepid, buf[stacd], 1460, TMO_FEVR )) <= E_OK )
//				break;
			// �p�P�b�g�����������\�������鎞�̎�M�菇�B�ȉ��̗��300ms�̃^�C���A�E�g�Ŕ��f
			// �ŏ��̃p�P�b�g�͉i�v�҂��Ŏ�M�v�����s���A�Q��ڈȍ~��300ms�̃^�C���A�E�g��ݒ�
			len = tcp_rcv_dat( cepid, buf[stacd], 1460, TMO_FEVR );
			while( (ercd=tcp_rcv_dat( cepid, &buf[stacd][len], 1460, 30 )) > E_OK )
				len += ercd;
			if( ercd != E_TMOUT )					// �G���[�R�[�h���^�C���A�E�g�łȂ���ΒʐM�I��
				break;						// �������[�v�𔲂��o��
			buf[stacd][len] = '\0';					// ��M�o�b�t�@�Ƀk���R�[�h��ݒ�
			tm_printf("cepid = %d : %s\n", cepid, buf[stacd] );	// �^�[�~�i���ɕ\��
			tcp_snd_dat( cepid, buf[stacd], len, TMO_FEVR );	// �N���C�A���g�ɃG�R�[�o�b�N
		}
		tcp_sht_cep( cepid );						// �ؒf����
		tcp_cls_cep( cepid, TMO_FEVR );					// �ʐM��ؒf
	}
}

void usermain(void)
{
int i;
ID objid;
T_CTSK t_ctsk;
	if( lan_open( ) != E_OK )				// LAN�\�P�b�g�̓���J�n
		goto Err;
	if( tcpudp_get_ramsize( ) > sizeof( tcpudp_work ) )	// ���[�N�G���A�̃T�C�Y���m�F
		goto Err;
	if( tcpudp_open( tcpudp_work ) != E_OK )		// TCP/UDP�ʐM���J�n
		goto Err;
	t_ctsk.exinf = (void*)1;				// �󂯌��|�[�gID���g�����ɐݒ�
	// �󂯌��|�[�gID(1)�̏ڍׂ� config_tcpudp.h �� CFG_TCP_REPID1_PORT_NUMBER �Œ�`
	t_ctsk.tskatr = TA_HLNG | TA_DSNAME;			// �^�X�N�̑�����ݒ�
	t_ctsk.stksz = 512;					// �^�X�N�̃X�^�b�N�T�C�Y��ݒ�
	t_ctsk.itskpri = 10;					// �^�X�N�̗D��x��ݒ�i�C�Ӂj
	t_ctsk.task =  server_tsk;				// �^�X�N�̋N���Ԓn��ݒ�
	strcpy( t_ctsk.dsname, "server" );			// �^�X�N�̃f�o�b�K�T�|�[�g����ݒ�
	for( i=0 ; i<3 ; i++ )  {				// �p�ӂ����ʐM�[�_�����̃��[�v
		// �p�ӂ����ʐM�[�_�̌��� config_tcpudp.h �� CFG_TCP_CEPID_NUM �Œ�`
		if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )	// �T�[�o�[�^�X�N�̐���
			goto Err;
		if( tk_sta_tsk( objid, i ) < E_OK )		// �T�[�o�[�^�X�N�̋N��
			goto Err;				// �ʐM�[�_ID�̓^�X�N�̋N���R�[�h�ɐݒ�
	}
	tk_slp_tsk( TMO_FEVR );					// �N���҂��ƂȂ�A�T�[�o�[�^�X�N�ɐ����n��
	tcpudp_close( );					// TCP/UDP�ʐM���I��
	lan_close( );						// LAN�\�P�b�g�̓����~
Err:
	while( 1 )  ;
}
