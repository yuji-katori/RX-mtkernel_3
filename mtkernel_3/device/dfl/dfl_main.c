/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dfl_main.c
 *
 *	Data Flash Interface Driver
 */

#include <tk/tkernel.h>
#include <dev_dfl.h>

LOCAL ER dfl_open(ID devid, UINT omode, void *exinf)
{
	return DFL_Open( );
}

LOCAL ER dfl_close(ID devid, UINT option, void *exinf)
{
	return DFL_Close( );
}

LOCAL ER dfl_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
	if( devreq->start < 0 || devreq->start & 3 || devreq->size <= 0
	 || devreq->start + devreq->size * sizeof(UW) > DFL_GetFlashSize( ) )	// Check Parameter
		devreq->error = E_PAR;						// Set Error Code
	else if( devreq->cmd == TDC_READ )
		DFL_Read( devreq );
	else
		DFL_Write( devreq );
	return E_OK;
}

LOCAL INT dfl_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
	return 0;
}

LOCAL ER dfl_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
	return E_OK;
}

LOCAL INT dfl_event(INT evttyp, void *evtinf, void *exinf)
{
UW fsaddr, feaddr;
	if( evttyp > TDV_BLOCKERASE256 )					// Check Event Type
		return E_PAR;							// Parameter Error
	fsaddr = (UW)evtinf;							// Set Start Address
	if( evttyp <= TDV_BLANKCHECK64 )  {					// Blank Check
		evttyp = evttyp == TDV_BLANKCHECK4 ? 3 : 63;			// Adjust Alignment
		if( fsaddr & evttyp || fsaddr >= DFL_GetFlashSize( ) )		// Parameter Check
			return E_PAR;						// Parameter Error
		feaddr = fsaddr + evttyp - 3;					// Set End Address
		return DFL_Blank_Check( fsaddr, feaddr );			// Blank Check
	}
	else  {									// Block Erase
		evttyp = ( 64 << ( evttyp - 2 ) ) - 1;				// Adjust Alignment
		if( fsaddr & evttyp || fsaddr >= DFL_GetFlashSize( ) )		// Parameter Check
			return E_PAR;						// Parameter Error
		feaddr = fsaddr + evttyp - 3;					// Set End Address
		return DFL_Block_Erase( fsaddr, feaddr );			// Block Erase
	}
}

EXPORT ER dflDrvEntry(void)
{
T_DDEV t_ddev;
	t_ddev.exinf = 0;					// Set Extend Information
	t_ddev.drvatr = 0;					// Set Driver Attribute
	t_ddev.nsub = 0;					// Set Sub Unit Number
	t_ddev.blksz = 4;					// Set Block Size
#ifdef CLANGSPEC
	t_ddev.openfn  = dfl_open;				// Set Open function Address
	t_ddev.closefn = dfl_close;				// Set Close function Address
	t_ddev.execfn  = dfl_exec;				// Set Execute function Address
	t_ddev.waitfn  = dfl_wait;				// Set Wait function Address
	t_ddev.abortfn = dfl_abort;				// Set Abort function Address
	t_ddev.eventfn = dfl_event;				// Set Event function Address
	return tk_def_dev( DFL_DEVNM, &t_ddev, NULL );		// Define Device Driver
#else
	t_ddev.openfn  = (FP)dfl_open;				// Set Open function Address
	t_ddev.closefn = (FP)dfl_close;				// Set Close function Address
	t_ddev.execfn  = (FP)dfl_exec;				// Set Execute function Address
	t_ddev.waitfn  = (FP)dfl_wait;				// Set Wait function Address
	t_ddev.abortfn = (FP)dfl_abort;				// Set Abort function Address
	t_ddev.eventfn = (FP)dfl_event;				// Set Event function Address
	return tk_def_dev( (UB*)DFL_DEVNM, &t_ddev, NULL );	// Define Device Driver
#endif	/* CLANGSPEC */
}