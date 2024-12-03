/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	tft_main.c
 *
 *	TFT Display Driver
 */

#include <tk/tkernel.h>
#include <dev_tft.h>

LOCAL W lock;

LOCAL ER tft_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER tft_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER tft_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
	if( devreq->abort )							// Abort Request ?
		devreq->error = E_ABORT;					// Set Error Code
	else if( devreq->cmd == TDC_READ )					// Command is Read ?
		devreq->error = E_NOSPT;					// No Support Error
	else									// Write Command
		if( devreq->start || devreq->size < 0 || devreq->buf == NULL )	// Check Parameter
			devreq->error = E_PAR;					// Parameter Error
		else  {								//
			drv_lock( TRUE, &lock );				// Lock
			devreq->asize = TFT_Write( devreq->size, devreq->buf );	// LCD Write
			drv_lock( FALSE, &lock );				// UnLock
			devreq->error = E_OK;					// Set Error Code
		}
	return E_OK;
}

LOCAL INT tft_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
	return E_OK;
}

LOCAL ER tft_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
	return E_OK;
}

LOCAL INT tft_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT ER tftDrvEntry(void)
{
T_DDEV t_ddev;

	TFT_Init( );					// TFT Initialize
	t_ddev.exinf = NULL;				// Set Extend Information
	t_ddev.drvatr = 0;				// Set Driver Attribute
	t_ddev.nsub = 0;				// Set Sub Unit Number
	t_ddev.blksz = 1;				// Set Block Size
#ifdef CLANGSPEC
	t_ddev.openfn  = tft_open;			// Set Open function Address
	t_ddev.closefn = tft_close;			// Set Close function Address
	t_ddev.execfn  = tft_exec;			// Set Execute function Address
	t_ddev.waitfn  = tft_wait;			// Set Wait function Address
	t_ddev.abortfn = tft_abort;			// Set Abort function Address
	t_ddev.eventfn = tft_event;			// Set Event function Address
	return tk_def_dev( TFT_DEVNM, &t_ddev, NULL );
#else
	t_ddev.openfn  = (FP)tft_open;			// Set Open function Address
	t_ddev.closefn = (FP)tft_close;			// Set Close function Address
	t_ddev.execfn  = (FP)tft_exec;			// Set Execute function Address
	t_ddev.waitfn  = (FP)tft_wait;			// Set Wait function Address
	t_ddev.abortfn = (FP)tft_abort;			// Set Abort function Address
	t_ddev.eventfn = (FP)tft_event;			// Set Event function Address
	return tk_def_dev( (UB*)TFT_DEVNM, &t_ddev, NULL );
#endif	/* CLANGSPEC */
}