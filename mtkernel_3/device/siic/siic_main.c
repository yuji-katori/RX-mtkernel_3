/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2023 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2024/3/11.
 *    Modified by Yuji Katori at 2024/4/1.
 *----------------------------------------------------------------------
 */

/*
 *	siic_main.c
 *
 *	Simple IIC Driver
 */

#include <tk/tkernel.h>
#include <dev_siic.h>

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
SIIC_TBL *siic = &siic_tbl[(UINT)exinf];
ER ercd;
	drv_lock( TRUE, &siic->lock );					// Lock
	if( siic->now & siic->next )					// Check Request Count
		ercd = E_LIMIT;						// Over Request Count
	else  {
		siic->now |= (UINT)devreq->exinf = siic->next;		// Set Flag Pattern
		if( ( siic->next >>= 1 ) == SIIC_MINIMUM )		// Minimum Value ?
			siic->next = SIIC_MAXIMUM;			// Set Maximum Value
		siic->req[siic->wt] = devreq;				// Set New Device Request
		if( ++ siic->wt == CFN_MAX_REQDEV+1 )			// Write Pointer is Max ?
			siic->wt = 0;					// Clear Write Pointer
		tk_set_flg( siic->flgid, SIIC_EXECCMD );		// Wakeup siic_tsk
		ercd = E_OK;						// Normal return
	}
	drv_lock( FALSE, &siic->lock );					// Unlock
	return ercd;
}

LOCAL INT siic_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UINT)exinf];
UINT flgptn;
ER ercd;
	ercd = tk_wai_flg( siic->flgid, (UINT)devreq->exinf, TWF_ORW | TWF_BITCLR, &flgptn, tmout );
	devreq->exinf = NULL;						// Clear Flag Pattern
	return ercd;							// Return
}

LOCAL ER siic_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UINT)exinf];
	drv_lock( TRUE, &siic->lock );					// Lock
	siic->now &= ~((UINT)devreq->exinf);				// Clear Flag Pattern
	tk_set_flg( siic->flgid, (UINT)devreq->exinf );			// Wakeup Request Task
	devreq->exinf = NULL;						// Clear Flag Pattern
	devreq->error = E_ABORT;					// Set Error Code
	drv_lock( FALSE, &siic->lock );					// Unlock
	return E_OK;
}

LOCAL INT siic_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT void siic_tsk(INT stacd, void *exinf)
{
SIIC_TBL *siic = &siic_tbl[(UINT)exinf];
UINT flgptn;
T_DEVREQ *devreq;
	while( 1 )  {
		tk_wai_flg( siic->flgid, SIIC_EXECCMD, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		drv_lock( TRUE, &siic->lock );				// Lock
		devreq = siic->req[siic->rd];				// Read Device Request
		if( ++ siic->rd == CFN_MAX_REQDEV+1 )			// Read Pointer is Max ?
			siic->rd = 0;					// Clear Read Pointer
		if( siic->wt != siic->rd )				// Nothing Device Request ?
			tk_set_flg( siic->flgid, SIIC_EXECCMD );	// Set Execute Command Bit
		drv_lock( FALSE, &siic->lock );				// Unlock
		if( devreq->abort )					// Abort Request ?
			devreq->error = E_ABORT;			// Set Error Code
		else if( devreq->cmd == TDC_READ )			// Command is Read ?
			if( devreq->size <= 0 )				// Check Size Parameter
				devreq->error = E_PAR;			// Parameter Error
			else						// Renesas I2C Read
				devreq->error = SIIC_Read( devreq, exinf );
		else							// Write Command
			if( devreq->size < 0 )				// Check Size Parameter
				devreq->error = E_PAR;			// Parameter Error
			else						// Renesas I2C Write
				devreq->error = SIIC_Write( devreq, exinf );
		drv_lock( TRUE, &siic->lock );				// Lock
		siic->now &= ~((UINT)devreq->exinf);			// Clear Flag Pattern
		tk_set_flg( siic->flgid, (UINT)devreq->exinf );		// Wakeup Request Task
		drv_lock( FALSE, &siic->lock );				// Unlock
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
