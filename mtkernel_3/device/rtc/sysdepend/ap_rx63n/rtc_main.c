/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	rtc_main.c
 *
 *	RTC Driver
 */

#include <tk/tkernel.h>
#include <dev_rtc.h>
#include "iodefine.h"

#define	BCDtoDEC( x )	((x) / 16 * 10 + (x) % 16)
#define	DECtoBCD( x )	((x) / 10 * 16 + (x) % 10)

LOCAL ER rtc_open(ID devid, UINT omode, void *exinf)
{
	RTC.RCR4.BYTE = 0x00;				// Select Sub Clock
	RTC.RCR2.BIT.START = 0;				// Stop RTC
	while( RTC.RCR2.BIT.START )  ;			// Wait RTC Stop
	RTC.RCR2.BIT.RESET = 1;				// Reset RTC
	while( RTC.RCR2.BIT.RESET )  ;			// Wait RTC Reset
	RTC.RCR2.BIT.HR24 = 1;				// 24H Mode
	RTC.RYRCNT.WORD  = 0;				// 1980
	RTC.RMONCNT.BYTE = 1;				// 1
	RTC.RDAYCNT.BYTE = 1;				// 1
	RTC.RWKCNT.BYTE  = 2;				// 0 is SUN
	RTC.RHRCNT.BYTE  = 0;				// 0
	RTC.RMINCNT.BYTE = 0;				// 0
	RTC.RWKCNT.BYTE  = 0;				// 0
	RTC.RCR2.BIT.START = 1;				// Start RTC
	while( !RTC.RCR2.BIT.START )  ;			// Wait RTC Start
	return E_OK;
}

LOCAL ER rtc_close(ID devid, UINT option, void *exinf)
{
	RTC.RCR2.BIT.START = 0;				// Stop RTC
	return E_OK;
}

LOCAL ER rtc_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
DATE_TIM *dt;
	switch( devreq->start )  {
	case DN_CKDATETIME:
		if( devreq->buf != NULL && devreq->size == sizeof(DATE_TIM) )  {
			dt = devreq->buf;
			devreq->asize = sizeof(DATE_TIM);
			if( devreq->cmd == TDC_READ )  {
				dt->d_year  = BCDtoDEC( RTC.RYRCNT.WORD ) + 80;
				dt->d_month = BCDtoDEC( RTC.RMONCNT.BYTE );
				dt->d_day   = BCDtoDEC( RTC.RDAYCNT.BYTE );
				dt->d_hour  = BCDtoDEC( RTC.RHRCNT.BYTE );
				dt->d_min   = BCDtoDEC( RTC.RMINCNT.BYTE );
				dt->d_sec   = BCDtoDEC( RTC.RSECCNT.BYTE );
				dt->d_week  = 0;
				dt->d_wday  = RTC.RWKCNT.BYTE;
				dt->d_days  = 0;
			}
			else	{
				RTC.RYRCNT.WORD  = DECtoBCD( dt->d_year - 80 );
				RTC.RMONCNT.BYTE = DECtoBCD( dt->d_month );
				RTC.RDAYCNT.BYTE = DECtoBCD( dt->d_day );
				RTC.RWKCNT.BYTE  = dt->d_wday;
				RTC.RHRCNT.BYTE  = DECtoBCD( dt->d_hour );
				RTC.RMINCNT.BYTE = DECtoBCD( dt->d_min );
				RTC.RSECCNT.BYTE = DECtoBCD( dt->d_sec );
			}
			return E_OK;
		}
		break;
	default:
		return E_NOSPT;
	}
	return E_PAR;
}

LOCAL INT rtc_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
	return 0;
}

LOCAL ER rtc_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
	return E_OK;
}

LOCAL INT rtc_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT ER rtcDrvEntry(void)
{
int i;
T_DDEV t_ddev;

	tk_dis_dsp( );					// Dispatch Disable
	SYSTEM.PRCR.WORD = 0xA503;			// Protect Disable
	if( SYSTEM.SOSCCR.BYTE )  {			// Sub-Clock Stoped ?
		SYSTEM.SOSCWTCR.BYTE = 0;		// tSUBOSC is 2.6 seconds(Suitable).
		SYSTEM.SOSCCR.BYTE = 0x00;		// Sub-Clock Stabilize
		for( i=0 ; i<62400000  ; i++ )  ;	// 2.6s/96MHz/4cyc=62400000
	}
	SYSTEM.PRCR.WORD = 0xA500;			// Protect Enable
	tk_ena_dsp( );					// Dispatch Enable

	t_ddev.exinf = NULL;				// Set Extend Information
	t_ddev.drvatr = 0;				// Set Driver Attribute
	t_ddev.nsub = 0;				// Set Sub Unit Number
	t_ddev.blksz = 0;				// Set Block Size
#ifdef CLANGSPEC
	t_ddev.openfn  = rtc_open;			// Set Open function Address
	t_ddev.closefn = rtc_close;			// Set Close function Address
	t_ddev.execfn  = rtc_exec;			// Set Execute function Address
	t_ddev.waitfn  = rtc_wait;			// Set Wait function Address
	t_ddev.abortfn = rtc_abort;			// Set Abort function Address
	t_ddev.eventfn = rtc_event;			// Set Event function Address
	return tk_def_dev( RTC_DEVNM, &t_ddev, NULL );
#else
	t_ddev.openfn  = (FP)rtc_open;			// Set Open function Address
	t_ddev.closefn = (FP)rtc_close;			// Set Close function Address
	t_ddev.execfn  = (FP)rtc_exec;			// Set Execute function Address
	t_ddev.waitfn  = (FP)rtc_wait;			// Set Wait function Address
	t_ddev.abortfn = (FP)rtc_abort;			// Set Abort function Address
	t_ddev.eventfn = (FP)rtc_event;			// Set Event function Address
	return tk_def_dev( (UB*)RTC_DEVNM, &t_ddev, NULL );
#endif	/* CLANGSPEC */
}