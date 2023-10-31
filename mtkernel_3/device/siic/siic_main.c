/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	siic_main.c
 *
 *	Simple IIC Driver
 */

#include <tk/tkernel.h>
#include <dev_siic.h>

LOCAL void sii_lock(INT mode, SIIC_TBL *siic)
{
W lock;
	if( mode == TRUE )  {						// Lock Process
		lock = TRUE;						// Set Lock Value
		while( __xchg( &siic->lock, &lock ), lock == TRUE )	// Wait Unlock
			tk_dly_tsk( 1 );				// Wait 1ms
	}
	else  {								// Unlock Process
		lock = FALSE;						// Set Unlock Value
		__xchg(	&siic->lock, &lock );				// Unlock
	}
}

LOCAL ER siic_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER siic_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER siic_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UB)exinf];
ER ercd;
	sii_lock( TRUE, siic );							// Lock
	if( siic->now & siic->next )						// Check Request Count
		ercd = E_LIMIT;							// Over Request Count
	else  {
		siic->now |= (UINT)devreq->exinf = siic->next;			// Set Flag Pattern
		if( ( siic->next >>= 1 ) == MINIMUM )				// Minimum Value ?
			siic->next = MAXIMUM;					// Set Maximum Value
		siic->req[siic->wt] = devreq;					// Set New Device Request
		if( ++ siic->wt == CFN_MAX_REQDEV+1 )				// Write Pointer is Max ?
			siic->wt = 0;						// Clear Write Pointer
		tk_set_flg( siic->flgid, EXECCMD );				// Wakeup sdc_tsk
		ercd = E_OK;							// Normal return
	}
	sii_lock( FALSE, siic );						// Unlock
	return ercd;
}

LOCAL INT siic_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UB)exinf];
UINT flgptn;
ER ercd;
	ercd = tk_wai_flg( siic->flgid, (UINT)devreq->exinf, TWF_ORW | TWF_BITCLR, &flgptn, tmout );
	devreq->exinf = NULL;							// Clear Flag Pattern
	return ercd;								// Return
}

LOCAL ER siic_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UB)exinf];
	sii_lock( TRUE, siic );							// Lock
	siic->now &= ~((UINT)devreq->exinf);					// Clear Flag Pattern
	tk_set_flg( siic->flgid, (UINT)devreq->exinf );				// Wakeup Request Task
	devreq->exinf = NULL;							// Clear Flag Pattern
	devreq->error = E_ABORT;						// Set Error Code
	sii_lock( FALSE, siic );						// Unlock
	return E_OK;
}

LOCAL INT siic_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT void siic_tsk(INT stacd, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UB)exinf];
UINT flgptn;
T_DEVREQ *devreq;
	while( 1 )  {
		tk_wai_flg( siic->flgid, TSK_WAIT_ALL, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		if( flgptn & EXECCMD )  {					// Execute Commond ?
			sii_lock( TRUE, siic );					// Lock
			devreq = siic->req[siic->rd];				// Read Device Request
			if( ++ siic->rd == CFN_MAX_REQDEV+1 )			// Read Pointer is Max ?
				siic->rd = 0;					// Clear Read Pointer
			if( siic->wt != siic->rd )				// Nothing Device Request ?
				tk_set_flg( siic->flgid, EXECCMD );		// Set Execute Command Bit
			sii_lock( FALSE, siic );				// Unlock
			if( devreq->abort )					// Abort request ?
				devreq->error = E_ABORT;			// Set Error Code
			else if( devreq->cmd == TDC_READ )			// Command is Read ?
				devreq->error = SIIC_Read( devreq, exinf );	// SD Card Read
			else							// Write Command
				devreq->error = SIIC_Write( devreq, exinf );	// SD Card Write
			sii_lock( TRUE, siic );					// Lock
			siic->now &= ~((UINT)devreq->exinf);			// Clear Flag Pattern
			tk_set_flg( siic->flgid, (UINT)devreq->exinf );		// Wakeup Request Task
			sii_lock( FALSE, siic );				// Unlock
		}
	}
}

EXPORT ER siicDriverEntry(VB *DrvName, UINT ch, T_DDEV *p_ddev)
{
	p_ddev->exinf = (void*)ch;				// Set Extend Information(Channel)
	p_ddev->drvatr = 0;					// Set Driver Attribute
	p_ddev->nsub = 0;					// Set Sub Unit Number
	p_ddev->blksz = 1;					// Set Block Size
#ifdef CLANGSPEC
	p_ddev->openfn  = siic_open;				// Set Open function Address
	p_ddev->closefn = siic_close;				// Set Close function Address
	p_ddev->execfn  = siic_exec;				// Set Execute function Address
	p_ddev->waitfn  = siic_wait;				// Set Wait function Address
	p_ddev->abortfn = siic_abort;				// Set Abort function Address
	p_ddev->eventfn = siic_event;				// Set Event function Address
	return tk_def_dev( DrvName, p_ddev, NULL );		// Define Device Driver
#else
	p_ddev->openfn  = (FP)siic_open;			// Set Open function Address
	p_ddev->closefn = (FP)siic_close;			// Set Close function Address
	p_ddev->execfn  = (FP)siic_exec;			// Set Execute function Address
	p_ddev->waitfn  = (FP)siic_wait;			// Set Wait function Address
	p_ddev->abortfn = (FP)siic_abort;			// Set Abort function Address
	p_ddev->eventfn = (FP)siic_event;			// Set Event function Address
	return tk_def_dev( (UB*)DrvName, p_ddev, NULL );	// Define Device Driver
#endif	/* CLANGSPEC */
}
