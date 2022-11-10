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
	RTC_OPEN( );
	return E_OK;
}

LOCAL ER rtc_close(ID devid, UINT option, void *exinf)
{
	RTC_CLOSE( );
	return E_OK;
}

LOCAL ER rtc_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
	switch( devreq->start )  {
	case DN_CKEVENT:
	case DN_CKAUTOPWON:
	case DN_CKREGISTER:
		return E_NOSPT;
	case DN_CKDATETIME:
		if( devreq->buf != NULL && devreq->size == sizeof(DATE_TIM) )  {
			DATE_TIM *dt = devreq->buf;
			if( devreq->cmd == TDC_READ )  {
				RTC_READ( );
			}
			else	{
				RTC_WRITE( );
			}
			devreq->asize = sizeof(DATE_TIM);
			devreq->error = E_OK;
			return E_OK;
		}
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
T_DDEV t_ddev;

	RTC_INIT( );

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