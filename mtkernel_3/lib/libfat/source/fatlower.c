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
}
#endif	/* _FS_READONLY */