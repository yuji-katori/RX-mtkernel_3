/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

#include <stdio.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>

LOCAL INT y, m, d, h, n, s;
LOCAL VB buf[32];
LOCAL SYSTIM tim;

void DATETIMtoTRON(void)
{
D  ll_tim, x;
INT i, z;
	ll_tim = 1000LL * ( s + 60 * ( n + 60L * ( h + ( d - 1 ) * 24 ) ) );
	if( y >= 2000 )  {				// 2000�N�ȍ~���H
		ll_tim += 946684800000;			// 30�N���̃J�E���g�l�����Z
		i = 2000;				// 2000�N���J�n�N�Ƃ���
	}
	else
		i = 1970;				// 1970�O���J�n�N�Ƃ���
	for(  ;  ; i++ )  {
		if( i % 4 )				// 4�Ŋ���؂�Ȃ����H
			// 11 --> 31day, 10 --> 30day, 01 --> 29day, 00 --> 28day
			// Dec Nov Oct Sep Aug Jul Jun May Apr Mar Feb Jan (Dec not used)
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// ���邤�N�ł͂Ȃ�
		else if( i % 100 )			// 100�Ŋ���؂�Ȃ����H
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// ���邤�N
		else if( i % 400 )			// 400�Ŋ���؂�Ȃ����H
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;	// ���邤�N�ł͂Ȃ�
		else					// 400�Ŋ���؂�
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;	// ���邤�N
		if( i == y )				// ���N���H
			break;
		ll_tim += x;				// �P�N���̕b�������Z
	}
	for( i=1 ; i!=m ; i++, z>>=2 )  {
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
		ll_tim += x;				// �P�P�����̕b�������Z
	}
	tim.hi = ll_tim >> 32;				// ���32�r�b�g��ݒ�
	tim.lo = ll_tim;				// ����32�r�b�g��ݒ�
}

void TRONtoDATETIM(void)
{
UW  l_tim;
D  ll_tim, x;
INT z;
	ll_tim = ( ( ll_tim = tim.hi ) << 32 ) + tim.lo;// UTC������64�r�b�g�ɕϊ�
	if( ll_tim < 946684800000 )			// 2000�N�ȉ��̃J�E���g�����H
		y = 1970;				// �J�n��1970�N
	else  {
		ll_tim -= 946684800000;			// 30�N���̃J�E���g�������Z
		for( y=2000 ;  ; y+=400 )  {		// �J�n��2000�N
			if( ll_tim < 12622780800000 )	// 400�N���̕b���ȉ����H
				break;
			ll_tim -= 12622780800000;	// 400�N���̃J�E���g�������Z
		}
		if( ll_tim >= 3155760000000 )  {	// 100�N�ȏ�̃J�E���g�����H
			ll_tim -= 3155760000000;	// 100�N���̃J�E���g�������Z
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
}

EXPORT INT usermain( void )
{
D ll_tim;
	tm_putstring("Input now date and time.\n"
		     "Year:Month:Day:Hour:Minute:Second\n"
		     "Ex. 2000:1:1:12:34:56\n\n");	// ���ݎ��������
	tm_getline( buf );
	sscanf( buf, "%ld:%ld:%ld:%ld:%ld:%ld", &y, &m, &d, &h, &n, &s );
	tm_putstring("\n");

	DATETIMtoTRON( );				// �N���������b��UTC�����ɕϊ�
	tk_set_utc( &tim );				// UTC������ݒ�
	while( 1 )  {
		tk_dly_tsk( 333 );			// 333ms�̑҂�
		tk_get_utc( &tim );			// UTC�������Q��
		TRONtoDATETIM( );			// UTC������N���������b�ɕϊ�
		tm_printf("\r%ld:%.2ld:%.2ld:%.2ld:%.2ld:%.2ld", y, m, d, h, n, s );
		ll_tim = ( ( ll_tim = tim.hi ) << 32 ) + tim.lo;	// UTC������64�r�b�g�ɕϊ�
		ll_tim /= 1000;
	}
}