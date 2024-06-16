/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	sci_main.c
 *
 *	Serial Communication Interface Driver
 */

#include <tk/tkernel.h>
#include <dev_sci.h>

LOCAL ER sci_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER sci_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER sci_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
SCI_TBL *sci = &sci_tbl[(UINT)exinf];
ER ercd;
	drv_lock( TRUE, &sci->lock );					// Lock
	if( sci->now & sci->next )					// Check Request Count
		ercd = E_LIMIT;						// Over Request Count
	else  {
		sci->now |= (UINT)devreq->exinf = sci->next;		// Set Flag Pattern
		if( ( sci->next >>= 1 ) == SCI_MINIMUM )		// Minimum Value ?
			sci->next = SCI_MAXIMUM;			// Set Maximum Value
		if( devreq->cmd == TDC_READ )  {			// Read Command ?
			sci->rreq[sci->rwt] = devreq;			// Set New Device Request
			if( ++ sci->rwt == CFN_MAX_REQDEV+1 )		// Receive Write Pointer is Max ?
				sci->rwt = 0;				// Clear Receive Write Pointer
			tk_set_flg( sci->flgid, SCI_RCVCMD );		// Wakeup sci_tsk at Receive Command
		}
		else  {							// Write Command
			sci->sreq[sci->swt] = devreq;			// Set New Device Request
			if( ++ sci->swt == CFN_MAX_REQDEV+1 )		// Receive Write Pointer is Max ?
				sci->swt = 0;				// Clear Receive Write Pointer
			tk_set_flg( sci->flgid, SCI_SNDCMD );		// Wakeup sci_tsk at Send Command
		}
		ercd = E_OK;						// Normal return
	}
	drv_lock( FALSE, &sci->lock );					// Unlock
	return ercd;
}

LOCAL INT sci_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
SCI_TBL *sci = &sci_tbl[(UINT)exinf];
UINT flgptn;
ER ercd;
	ercd = tk_wai_flg( sci->flgid, (UINT)devreq->exinf, TWF_ORW | TWF_BITCLR, &flgptn, tmout );
	devreq->exinf = NULL;						// Clear Flag Pattern
	return ercd;							// Return
}

LOCAL ER sci_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
SCI_TBL *sci = &sci_tbl[(UINT)exinf];
	drv_lock( TRUE, &sci->lock );					// Lock
	sci->now &= ~((UINT)devreq->exinf);				// Clear Flag Pattern
	tk_set_flg( sci->flgid, (UINT)devreq->exinf );			// Wakeup Request Task
	devreq->exinf = NULL;						// Clear Flag Pattern
	devreq->error = E_ABORT;					// Set Error Code
	drv_lock( FALSE, &sci->lock );					// Unlock
	return E_OK;
}

LOCAL INT sci_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT void sci_tsk(INT stacd, void *exinf)
{
SCI_TBL *sci = &sci_tbl[(UINT)exinf];
UINT flgptn;
T_DEVREQ *devreq;
	while( 1 )  {
		tk_wai_flg( sci->flgid, SCI_WAITALL, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		if( flgptn & SCI_RCVEND )  {				// Receive End Command ?
			drv_lock( TRUE, &sci->lock );			// Lock
			devreq = sci->rwreq;				// Set Device Request
			sci->rwreq = 0;					// Clear Device Request
			sci->now &= ~((UINT)devreq->exinf);		// Clear Flag Pattern
			tk_set_flg( sci->flgid, (UINT)devreq->exinf );	// Wakeup Request Task
			if( sci->rwt != sci->rrd )			// Nothing Read Device Request ?
				tk_set_flg( sci->flgid, SCI_RCVCMD );	// Set Receive Execute Command Bit
			drv_lock( FALSE, &sci->lock );			// Unlock
		}
		if( flgptn & SCI_SNDEND )  {				// Send End Command ?
			drv_lock( TRUE, &sci->lock );			// Lock
			devreq = sci->swreq;				// Set Device Request
			sci->swreq = 0;					// Clear Device Request
			sci->now &= ~((UINT)devreq->exinf);		// Clear Flag Pattern
			tk_set_flg( sci->flgid, (UINT)devreq->exinf );	// Wakeup Request Task
			if( sci->swt != sci->srd )			// Nothing Write Device Request ?
				tk_set_flg( sci->flgid, SCI_SNDCMD );	// Set Send Execute Command Bit
			drv_lock( FALSE, &sci->lock );			// Unlock
		}
		if( flgptn & SCI_RCVCMD )  {				// Receive Command ?
			devreq = 0;					// Clear Device Request
			drv_lock( TRUE, &sci->lock );			// Lock
			if( ! sci->rwreq )  {				// Not Exist Receive Request
				devreq = sci->rreq[sci->rrd];		// Read Device Request
				if( ++ sci->rrd == CFN_MAX_REQDEV+1 )	// Read Pointer is Max ?
					sci->rrd = 0;			// Clear Read Pointer
			}
			drv_lock( FALSE, &sci->lock );			// Unlock
			if( devreq && ! devreq->abort )  {		// Execute Receive Request ?
				sci->rwreq = devreq;			// Set Receive Device Request
				SCI_Read( devreq, exinf );		// SCI Read
			}
		}
		if( flgptn & SCI_SNDCMD )  {				// Send Command ?
			devreq = 0;					// Clear Device Request
			drv_lock( TRUE, &sci->lock );			// Lock
			if( ! sci->swreq )  {				// Not Exist Send Request
				devreq = sci->sreq[sci->srd];		// Read Device Request
				if( ++ sci->srd == CFN_MAX_REQDEV+1 )	// Read Pointer is Max ?
					sci->srd = 0;			// Clear Read Pointer
			}
			drv_lock( FALSE, &sci->lock );			// Unlock
			if( devreq && ! devreq->abort )  {		// Execute Send Request ?
				sci->swreq = devreq;			// Set Send Device Request
				SCI_Write( devreq, exinf );		// SCI Write
			}
		}
	}
}

EXPORT ER sciDriverEntry(VB *DrvName, UINT ch, T_DDEV *p_ddev)
{
	p_ddev->exinf = (void*)ch;				// Set Extend Information(Channel)
	p_ddev->drvatr = 0;					// Set Driver Attribute
	p_ddev->nsub = 0;					// Set Sub Unit Number
	p_ddev->blksz = 1;					// Set Block Size
#ifdef CLANGSPEC
	p_ddev->openfn  = sci_open;				// Set Open function Address
	p_ddev->closefn = sci_close;				// Set Close function Address
	p_ddev->execfn  = sci_exec;				// Set Execute function Address
	p_ddev->waitfn  = sci_wait;				// Set Wait function Address
	p_ddev->abortfn = sci_abort;				// Set Abort function Address
	p_ddev->eventfn = sci_event;				// Set Event function Address
	return tk_def_dev( DrvName, p_ddev, NULL );		// Define Device Driver
#else
	p_ddev->openfn  = (FP)sci_open;				// Set Open function Address
	p_ddev->closefn = (FP)sci_close;			// Set Close function Address
	p_ddev->execfn  = (FP)sci_exec;				// Set Execute function Address
	p_ddev->waitfn  = (FP)sci_wait;				// Set Wait function Address
	p_ddev->abortfn = (FP)sci_abort;			// Set Abort function Address
	p_ddev->eventfn = (FP)sci_event;			// Set Event function Address
	return tk_def_dev( (UB*)DrvName, p_ddev, NULL );	// Define Device Driver
#endif	/* CLANGSPEC */
}
