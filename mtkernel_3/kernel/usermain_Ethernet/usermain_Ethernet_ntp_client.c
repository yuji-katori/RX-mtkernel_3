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

// UTC�����̃I�[�o�t���[���Ǘ�����}�N�����ł��B�{�v���O�����N�����̎�����
// 2036�N2��6��6��28��15�b�𒴂��Ă��Ȃ��ꍇ�� 0 ���w�肵�܂��B
// 2036�N2��6��6��28��15�b�𒴂���ꍇ�� 1 ���w�肵�܂��B
// �Ȍ�A4,294,967,295�b�𒴂��閈�� +1 ���w�肵�܂��B
#define UTCBASE		0	// ���݂�2036�N2��6��6��28��15�b ����
// ���{�̕W���������g�p���邩�A�O���j�b�`�W���������g�p���邩���w�肵�܂��B
// �O���j�b�`�W���������g�p����ꍇ�͈ȉ��̃}�N�������R�����g�A�E�g���܂��B
#define JPNTIME			// ���{�̕W���������g�p
// NTP�T�[�o��IP�A�h���X�ł��B�P�����}�N������L���ɂ��܂��B
#define NTPSERVER	0x85F3EEF3	// 133.243.238.243 NICT(�Ɨ��s���@�l���ʐM�����@�\)
//#define NTPSERVER	0x850F4008	// 133. 15. 64.  8 �L���Z�p�Ȋw��w

void UTCtoTS(W hi, UW lo, UW *s, UW *f);
void TStoUTC(UW s, UW f, W *hi, UW *lo);
void UTCtoDATETIM(W hi, UW lo);

#define T4_WORK_SIZE (2796)
LOCAL UW tcpudp_work[(T4_WORK_SIZE / sizeof(UW))];

typedef struct {		// NTP�p�P�b�g�\��
	UB	li_vn_mode, stratum, poll, precision;
	UW	rootDelay, rootDispersion, refId;
	UW	refTm_s, refTm_f,  origTm_s, origTm_f,  rxTm_s, rxTm_f,  txTm_s, txTm_f;
} NTP_PAKET;
LOCAL NTP_PAKET buf;		// ����M�o�b�t�@(NTP�p�P�b�g)

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
	dstaddr.ipaddr = NTPSERVER;					// �T�[�o��IP�A�h���X��ݒ�
	dstaddr.portno = 123;						// �T�[�o�̃|�[�g�ԍ���ݒ�
	// ���ǂ̃|�[�g�ԍ���CFG_UDP_CEPID#_PORT_NUMBER��123���w��
	t1 = 4294967296000 * UTCBASE;					// UTC�̃x�[�X������ݒ�
	tim.hi = t1 >> 32;  tim.lo = t1;				// �V�X�e�������p�̕ϐ���������
	tk_set_utc( &tim );						// UTC������������
	while( 1 )  {
		buf.li_vn_mode = 0x23;					// LI, VN, MODE��ݒ�
		buf.poll = 17;						// Poll��ݒ�
		buf.stratum = buf.precision = buf.rootDelay = buf.rootDispersion = buf.refId = 0;
		tk_get_utc( &tim );					// UTC�������Q��
		UTCtoTS( tim.hi, tim.lo, &buf.txTm_s, &buf.txTm_f );	// ���M�̃^�C���X�^���v��ݒ�
		t1 = ( ( t1 = tim.hi ) << 32 ) + tim.lo;		// t1��UTC����(64�r�b�g)��ݒ�
		if( udp_snd_dat( cepid, &dstaddr, &buf, sizeof(buf), 3000 ) != sizeof(buf) )
			break;						// �T�[�o�փf�[�^���M
		if( udp_rcv_dat( cepid, &dstaddr, &buf, sizeof(buf), 3000 ) != sizeof(buf) )
			break;						// �T�[�o����f�[�^��M
		tk_get_utc( &tim );					// t4��UTC�������Q��
		UTCtoTS( tim.hi, tim.lo, &buf.refTm_s, &buf.refTm_f );	// t4�̃^�C���X�^���v��ݒ�
		t4 = ( ( t4 = tim.hi ) << 32 ) + tim.lo;		// t4��UTC����(64�r�b�g)��ݒ�
		TStoUTC( buf.rxTm_s, buf.rxTm_f, &tim.hi, &tim.lo );	// t2��UTC�����ɕϊ�
		t2 = ( ( t2 = tim.hi ) << 32 ) + tim.lo;		// t2��UTC����(64�r�b�g)��ݒ�
		TStoUTC( buf.txTm_s, buf.txTm_f, &tim.hi, &tim.lo );	// t3��UTC�����ɕϊ�
		t3 = ( ( t3 = tim.hi ) << 32 ) + tim.lo;		// t3��UTC����(64�r�b�g)��ݒ�
		t1 = ( ( t2 - t1 ) + ( t3 - t4 ) ) / 2;			// �I�t�Z�b�g���Z�o
		tk_get_utc( &tim );					// UTC�������Q��
		t1 += t4 = ( ( t4 = tim.hi ) << 32 ) + tim.lo;		// ����UTC�������Z�o
		t1 += t4 = ( t4 / 1000 >> 32 << 32 ) * 1000;		// 2036�N����␳
		tim.hi = t1 >> 32;  tim.lo = t1;			// ����UTC������ݒ�
		tk_set_utc( &tim );					// ����UTC������ݒ�
		UTCtoDATETIM( tim.hi, tim.lo );				// UTC������\��
		buf.origTm_s = buf.txTm_s;  buf.origTm_f = buf.txTm_f;	// �����t3���o�b�t�@�ɐݒ�
		buf.rxTm_s   = buf.refTm_s; buf.rxTm_f   = buf.refTm_f;	// �����t4���o�b�t�@�ɐݒ�
		UTCtoTS( tim.hi, tim.lo, &buf.refTm_s, &buf.refTm_f );	// ����UTC�����̃^�C���X�^���v��ݒ�
		tk_dly_tsk( 1000 * 60 * 60 * 24 );			// 24���ԑ҂�
	}
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
	t_ctsk.stksz = 356;					// �^�X�N�̃X�^�b�N�T�C�Y��ݒ�
	t_ctsk.itskpri = 10;					// �^�X�N�̗D��x��ݒ�i�C�Ӂj
	t_ctsk.task =  ntp_client_tsk;				// �^�X�N�̋N���Ԓn��ݒ�
	strcpy( t_ctsk.dsname, "ntp_cli" );			// �^�X�N�̃f�o�b�K�T�|�[�g����ݒ�
	if( (objid = tk_cre_tsk( &t_ctsk )) <= E_OK )		// �N���C�A���g�^�X�N�̐���
		goto Err;
	if( tk_sta_tsk( objid, 1 ) < E_OK )			// �N���C�A���g�^�X�N�̋N��
		goto Err;					// �ʐM�[�_ID�̓^�X�N�̋N���R�[�h�ɐݒ�
	tk_slp_tsk( TMO_FEVR );					// �ʐM�̏I����҂�
	tcpudp_close( );					// TCP/UDP�ʐM���I��
	lan_close( );						// LAN�\�P�b�g�̓����~
Err:
	while( 1 )  ;
}


void UTCtoTS(W hi, UW lo, UW *s, UW *f)
{
D ll_tim;
	ll_tim = ( ( ll_tim = hi ) << 32 ) + lo;		// UTC������64�r�b�g�ɕϊ�
	*s = TS( ll_tim / 1000 );				// �^�C���X�^���v�̕b����ݒ�
	*f = TS( ( ll_tim % 1000 << 32 ) / 1000 );		// �^�C���X�^���v�̒[����ݒ�
}

void TStoUTC(UW s, UW f, W *hi, UW *lo)
{
D ll_tim;
	ll_tim  = TS( s ) * 1000LL + ( TS( f ) * 1000LL >> 32 );// �^�C���X�^���v��64�r�b�g�ɕϊ�
	*hi = ll_tim >> 32;					// UTC�����̏��32�r�b�g��ݒ�
	*lo = ll_tim;						// UTC�����̉���32�r�b�g��ݒ�
}

void UTCtoDATETIM(W hi, UW lo)
{
UW  l_tim;
D  ll_tim, x;
INT y, m, d, h, n, s, z;
	ll_tim = ( ( ll_tim = hi ) << 32 ) + lo;		// UTC������64�r�b�g�ɕϊ�
#ifdef JPNTIME							// ���{�̕W���������H
	ll_tim += 32400000;					// �����X���ԕ������Z
#endif
	if( ll_tim < 3155673600000 )				// 2000�N�����̃J�E���g�����H
		y = 1900;					// �J�n��1900�N
	else  {
		ll_tim -= 3155673600000;			// 100�N���̃J�E���g�������Z
		for( y=2000 ;  ; y+=400 )  {			// �J�n��2000�N
			if( ll_tim < 12622780800000 )		// 400�N���̕b���ȉ����H
				break;
			ll_tim -= 12622780800000;		// 400�N���̃J�E���g�������Z
		}
		if( ll_tim >= 3155760000000 )  {		// 100�N�ȏ�̃J�E���g�����H
			ll_tim -= 3155760000000;		// 100�N���̃J�E���g�������Z
			for(  ;  ; y+=100 )  {
				if( ll_tim < 3155673600000 )	// 100�N�����̃J�E���g�����H
					break;
				ll_tim -= 3155673600000;	// 100�N���̃J�E���g�������Z
			}
		}
	}
	for(  ;  ; y++ )  {
		if( y % 4 )				// 4�Ŋ���؂�Ȃ����H
			// 11 --> 31day, 10 --> 30day, 01 --> 29day, 00 --> 28day
			// Dec Nov Oct Sep Aug Jul Jun May Apr Mar Feb Jan (Dec not used)
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// ���邤�N�ł͂Ȃ�
		else if( y % 100 )			// 100�Ŋ���؂�Ȃ����H
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// ���邤�N
		else if( y % 400 )			// 400�Ŋ���؂�Ȃ����H
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// ���邤�N�ł͂Ȃ�
		else					// 400�Ŋ���؂�
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// ���邤�N
		if( ll_tim < x )			// �c��̕b���͂P�N�ȓ����H
			break;
		ll_tim -= x;				// �P�N���̕b�������Z
	}
	for( m=1 ;  ; m++, z>>=2 )  {
		if( z & 2 )				// 31����30�����H
			if( z & 1 )			// 31�����H
				x = 2678400000;		// 31��
			else
				x = 2592000000;		// 30��
		else
			if( z & 1 )			// 29����28�����H
				x = 2505600000;		// 29��
			else
				x = 2419200000;		// 28��
		if( ll_tim < x )			// �c��̕b���͂P�P���ȓ����H
			break;
		ll_tim -= x;				// �P�P�����̕b�������Z
	}
	l_tim = ll_tim;					// long long�^����long�^�ɕϊ�
	d = l_tim / 86400000 + 1, l_tim %= 86400000;	// �������Z�o
	h = l_tim / 3600000     , l_tim %= 3600000;	// �������Z�o
	n = l_tim / 60000       , l_tim %= 60000;	// �������Z�o
	s = l_tim / 1000;				// �b�����Z�o
	tm_printf("\r%ld:%.2ld:%.2ld:%.2ld:%.2ld:%.2ld\n", y, m, d, h, n, s);
}