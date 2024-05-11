/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	iic_main.c
 *
 *	Renesas IIC(RIIC) Driver
 */

#include <tk/tkernel.h>
#include <dev_iic.h>

LOCAL ER iic_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER iic_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER iic_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
IIC_TBL *iic = &iic_tbl[(UINT)exinf];
ER ercd;
	drv_lock( TRUE, &iic->lock );					// Lock
	if( iic->now & iic->next )					// Check Request Count
		ercd = E_LIMIT;						// Over Request Count
	else  {
		iic->now |= (UINT)devreq->exinf = iic->next;		// Set Flag Pattern
		if( ( iic->next >>= 1 ) == IIC_MINIMUM )		// Minimum Value ?
			iic->next = IIC_MAXIMUM;			// Set Maximum Value
		iic->req[iic->wt] = devreq;				// Set New Device Request
		if( ++ iic->wt == CFN_MAX_REQDEV+1 )			// Write Pointer is Max ?
			iic->wt = 0;					// Clear Write Pointer
		tk_set_flg( iic->flgid, IIC_EXECCMD );			// Wakeup iic_tsk
		ercd = E_OK;						// Normal return
	}
	drv_lock( FALSE, &iic->lock );					// Unlock
	return ercd;
}

LOCAL INT iic_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
IIC_TBL *iic = &iic_tbl[(UINT)exinf];
UINT flgptn;
ER ercd;
	ercd = tk_wai_flg( iic->flgid, (UINT)devreq->exinf, TWF_ORW | TWF_BITCLR, &flgptn, tmout );
	devreq->exinf = NULL;						// Clear Flag Pattern
	return ercd;							// Return
}

LOCAL ER iic_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
IIC_TBL *iic = &iic_tbl[(UINT)exinf];
	drv_lock( TRUE, &iic->lock );					// Lock
	iic->now &= ~((UINT)devreq->exinf);				// Clear Flag Pattern
	tk_set_flg( iic->flgid, (UINT)devreq->exinf );			// Wakeup Request Task
	devreq->exinf = NULL;						// Clear Flag Pattern
	devreq->error = E_ABORT;					// Set Error Code
	drv_lock( FALSE, &iic->lock );					// Unlock
	return E_OK;
}

LOCAL INT iic_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT void iic_tsk(INT stacd, void *exinf)
{
IIC_TBL *iic = &iic_tbl[(UINT)exinf];
UINT flgptn;
T_DEVREQ *devreq;
	while( 1 )  {
		tk_wai_flg( iic->flgid, IIC_EXECCMD, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		drv_lock( TRUE, &iic->lock );				// Lock
		devreq = iic->req[iic->rd];				// Read Device Request
		if( ++ iic->rd == CFN_MAX_REQDEV+1 )			// Read Pointer is Max ?
			iic->rd = 0;					// Clear Read Pointer
		if( iic->wt != iic->rd )				// Nothing Device Request ?
			tk_set_flg( iic->flgid, IIC_EXECCMD );		// Set Execute Command Bit
		drv_lock( FALSE, &iic->lock );				// Unlock
		if( devreq->abort )					// Abort Request ?
			devreq->error = E_ABORT;			// Set Error Code
		else if( devreq->cmd == TDC_READ )			// Command is Read ?
			if( devreq->size <= 0 )				// Check Size Parameter
				devreq->error = E_PAR;			// Parameter Error
			else						// Renesas I2C Read
				devreq->error = IIC_Read( devreq, exinf );
		else							// Write Command
			if( devreq->size < 0 )				// Check Size Parameter
				devreq->error = E_PAR;			// Parameter Error
			else						// Renesas I2C Write
				devreq->error = IIC_Write( devreq, exinf );
		drv_lock( TRUE, &iic->lock );				// Lock
		iic->now &= ~((UINT)devreq->exinf);			// Clear Flag Pattern
		tk_set_flg( iic->flgid, (UINT)devreq->exinf );		// Wakeup Request Task
		drv_lock( FALSE, &iic->lock );				// Unlock
	}
}

EXPORT ER iicDriverEntry(VB *DrvName, UINT ch, T_DDEV *p_ddev)
{
	p_ddev->exinf = (void*)ch;				// Set Extend Information(Channel)
	p_ddev->drvatr = 0;					// Set Driver Attribute
	p_ddev->nsub = 0;					// Set Sub Unit Number
	p_ddev->blksz = 1;					// Set Block Size
#ifdef CLANGSPEC
	p_ddev->openfn  = iic_open;				// Set Open function Address
	p_ddev->closefn = iic_close;				// Set Close function Address
	p_ddev->execfn  = iic_exec;				// Set Execute function Address
	p_ddev->waitfn  = iic_wait;				// Set Wait function Address
	p_ddev->abortfn = iic_abort;				// Set Abort function Address
	p_ddev->eventfn = iic_event;				// Set Event function Address
	return tk_def_dev( DrvName, p_ddev, NULL );		// Define Device Driver
#else
	p_ddev->openfn  = (FP)iic_open;				// Set Open function Address
	p_ddev->closefn = (FP)iic_close;			// Set Close function Address
	p_ddev->execfn  = (FP)iic_exec;				// Set Execute function Address
	p_ddev->waitfn  = (FP)iic_wait;				// Set Wait function Address
	p_ddev->abortfn = (FP)iic_abort;			// Set Abort function Address
	p_ddev->eventfn = (FP)iic_event;			// Set Event function Address
	return tk_def_dev( (UB*)DrvName, p_ddev, NULL );	// Define Device Driver
#endif	/* CLANGSPEC */
}
