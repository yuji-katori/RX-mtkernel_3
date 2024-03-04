/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	fatlower.c
 */

#include <tk/tkernel.h>
#include <dev_rtc.h>
#include "ff.h"

#if _FS_READONLY == 0

DWORD get_fattime (void)
{

#if FF_FS_NORTC == 1
ID rtc;
ER ercd;
DATE_TIM time;
SZ asize;

#ifdef CLANGSPEC
	rtc = tk_opn_dev( RTC_DEVNM, TD_READ );
#else
	rtc = tk_opn_dev( (UB*)RTC_DEVNM, TD_READ );
#endif	/* CLANGSPEC */
	if( rtc >= E_OK )  {
		ercd = tk_srea_dev( rtc, DN_CKDATETIME, &time, sizeof(DATE_TIM), &asize );
		tk_cls_dev( rtc, 0 );
		if( ercd >= E_OK )  {
			return    ((DWORD)(time.d_year - 80) << 25)
				| ((DWORD) time.d_month      << 21)
				| ((DWORD) time.d_day 	     << 16)
				| ((DWORD) time.d_hour       << 11)
				| ((DWORD) time.d_min        <<  5)
				| ((DWORD) time.d_sec        >>  1);
		}
	}
	return	  ((DWORD) 20 << 25)	// 2000/1/1 0:0:0
		| ((DWORD)  1 << 21)
		| ((DWORD)  1 << 16)
		| ((DWORD)  0 << 11)
		| ((DWORD)  0 <<  5)
		| ((DWORD)  0 >>  1);
#elif FF_FS_NORTC == 2
SYSTIM tim;
UW  l_tim;
D  ll_tim, x;
INT z, y, m, d, h, n, s;
	tk_get_utc( &tim );					// UTC�������Q��
	ll_tim = ( ( ll_tim = tim.hi ) << 32 ) + tim.lo;	// UTC������64�r�b�g�ɕϊ�
	if( ll_tim < 946684800000 )				// 2000�N�ȉ��̃J�E���g�����H
		y = 1970;					// �J�n��1970�N
	else  {
		ll_tim -= 946684800000;				// 30�N���̃J�E���g�������Z
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
		if( y % 4 )					// 4�Ŋ���؂�Ȃ����H
			// 11 --> 31day, 10 --> 30day, 01 --> 29day, 00 --> 28day
			// Dec Nov Oct Sep Aug Jul Jun May Apr Mar Feb Jan (Dec not used)
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;		// ���邤�N�ł͂Ȃ�
		else if( y % 100 )				// 100�Ŋ���؂�Ȃ����H
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;		// ���邤�N
		else if( y % 400 )				// 400�Ŋ���؂�Ȃ����H
			// 11  10  11  10  11  11  10  11  10  11  00  11
			x = 31536000000, z = 0xEEFBB3;		// ���邤�N�ł͂Ȃ�
		else						// 400�Ŋ���؂�
			// 11  10  11  10  11  11  10  11  10  11  01  11
			x = 31622400000, z = 0xEEFBB7;		// ���邤�N
		if( ll_tim < x )				// �c��̕b���͂P�N�ȓ����H
			break;
		ll_tim -= x;					// �P�N���̕b�������Z
	}
	for( m=1 ;  ; m++, z>>=2 )  {
		if( z & 2 )					// 31����30�����H
			if( z & 1 )				// 31�����H
				x = 2678400000;			// 31��
			else
				x = 2592000000;			// 30��
		else
			if( z & 1 )				// 29����28�����H
				x = 2505600000;			// 29��
			else
				x = 2419200000;			// 28��
		if( ll_tim < x )				// �c��̕b���͂P�P���ȓ����H
			break;
		ll_tim -= x;					// �P�P�����̕b�������Z
	}
	l_tim = ll_tim;						// long long�^����long�^�ɕϊ�
	d = l_tim / 86400000 + 1, l_tim %= 86400000;		// �������Z�o
	h = l_tim / 3600000     , l_tim %= 3600000;		// �������Z�o
	n = l_tim / 60000       , l_tim %= 60000;		// �������Z�o
	s = l_tim / 1000;					// �b�����Z�o
	return    ((DWORD)(y - 1980) << 25)
		| ((DWORD) m         << 21)
		| ((DWORD) d 	     << 16)
		| ((DWORD) h         << 11)
		| ((DWORD) n         <<  5)
		| ((DWORD) s         >>  1);
#endif
}
#endif	/* _FS_READONLY */