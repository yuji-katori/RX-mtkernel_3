/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2024 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	qspi_main.c
 *
 *	Renesas QSPI(Serial Flash) Driver
 */

#include <string.h>
#include <tk/tkernel.h>
#include <dev_qspi.h>

LOCAL ID flgid;
LOCAL W lock;
LOCAL UB rd, wt;
LOCAL T_DEVREQ *req[CFN_MAX_REQDEV+1];
LOCAL UINT now, next = QSPI_MAXIMUM;
#if !USE_IMALLOC
LOCAL INT qspi_task_stack[260/sizeof(INT)];
#endif /* USE_IMALLOC */

LOCAL ER qspi_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER qspi_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER qspi_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
ER ercd;
	drv_lock( TRUE, &lock );				// Lock
	if( now & next )					// Check Request Count
		ercd = E_LIMIT;					// Over Request Count
	else  {
		now |= (UINT)devreq->exinf = next;		// Set Flag Pattern
		if( ( next >>= 1 ) == QSPI_MINIMUM )		// Minimum Value ?
			next = QSPI_MAXIMUM;			// Set Maximum Value
		req[wt] = devreq;				// Set New Device Request
		if( ++ wt == CFN_MAX_REQDEV+1 )			// Write Pointer is Max ?
			wt = 0;					// Clear Write Pointer
		tk_set_flg( flgid, QSPI_EXECCMD );		// Wakeup qspi_tsk
		ercd = E_OK;					// Normal return
	}
	drv_lock( FALSE, &lock );				// Unlock
	return ercd;
}

LOCAL INT qspi_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
UINT flgptn;
ER ercd;
	ercd = tk_wai_flg( flgid, (UINT)devreq->exinf, TWF_ORW | TWF_BITCLR, &flgptn, tmout );
	devreq->exinf = NULL;					// Clear Flag Pattern
	return ercd;						// Return
}

LOCAL ER qspi_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
	drv_lock( TRUE, &lock );				// Lock
	now &= ~((UINT)devreq->exinf);				// Clear Flag Pattern
	tk_set_flg( flgid, (UINT)devreq->exinf );		// Wakeup Request Task
	devreq->exinf = NULL;					// Clear Flag Pattern
	devreq->error = E_ABORT;				// Set Error Code
	drv_lock( FALSE, &lock );				// Unlock
	return E_OK;
}

LOCAL INT qspi_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT void qspi_tsk(INT stacd, void *exinf)
{
UINT flgptn;
T_DEVREQ *devreq;
TMO tmout = 1;
	while( 1 )  {
		if( E_TMOUT == tk_wai_flg( flgid, QSPI_WAITALL, TWF_ANDW | TWF_BITCLR, &flgptn, tmout ) )  {
			if( TMO_FEVR == ( tmout = QSPI_WtCheck( ) ) )	// Check Write Operation End
				tk_set_flg( flgid, QSPI_WRITEIP );	// Set Not Write in Process
			continue;					// Next 
		}
		drv_lock( TRUE, &lock );				// Lock
		devreq = req[rd];					// Read Device Request
		if( ++ rd == CFN_MAX_REQDEV+1 )				// Read Pointer is Max ?
			rd = 0;						// Clear Read Pointer
		if( wt != rd )						// Nothing Device Request ?
			tk_set_flg( flgid, QSPI_EXECCMD );		// Set Execute Command Bit
		drv_lock( FALSE, &lock );				// Unlock
		if( devreq->abort )					// Abort Request ?
			devreq->error = E_ABORT;			// Set Error Code
		else if( devreq->cmd == TDC_READ )  {			// Command is Read ?
			QSPI_Read( devreq );				// Execute Read Command
			tk_set_flg( flgid, QSPI_WRITEIP );		// Set Not Write in Process
		}							// Write Command
		else if( TMO_FEVR == ( tmout = QSPI_Write( devreq ) ) )	// Check Write Operation End
			tk_set_flg( flgid, QSPI_WRITEIP );		// Set Not Write in Process
		drv_lock( TRUE, &lock );				// Lock
		now &= ~((UINT)devreq->exinf);				// Clear Flag Pattern
		tk_set_flg( flgid, (UINT)devreq->exinf );		// Wakeup Request Task
		drv_lock( FALSE, &lock );				// Unlock
	}
}

EXPORT ER qspiDrvEntry(void)
{
ID objid;
union { T_CTSK t_ctsk; T_CFLG t_cflg; T_DDEV t_ddev; } u;

	u.t_ctsk.tskatr  = TA_HLNG;				// Set Task Attribute
#if USE_OBJECT_NAME
	u.t_ctsk.tskatr |= TA_DSNAME;				// Set Task Attribute
#endif /* USE_OBJECT_NAME */
#if !USE_IMALLOC
	u.t_ctsk.tskatr |= TA_USERBUF;				// Set Task Attribute
	u.t_ctsk.bufptr = qspi_task_stack;			// Set Stack Top Address
#endif /* USE_OBJECT_NAME */
	u.t_ctsk.stksz = 260;					// Set Task StackSize
	u.t_ctsk.itskpri = QSPI_GetTaskPri( );			// Set Task Priority
#ifdef CLANGSPEC
	u.t_ctsk.task =  qspi_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( u.t_ctsk.dsname, "qspi_t" );			// Set Task Debugger Suport Name
#endif /* USE_OBJECT_NAME */
#else
	u.t_ctsk.task =  (FP)qspi_tsk;				// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)u.t_ctsk.dsname, "qspi_t" );		// Set Task Debugger Suport Name
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( &u.t_ctsk )) <= E_OK )		// Create QSPI Control Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) < E_OK )			// Start QSPI Control Task
		goto ERROR;

	u.t_cflg.flgatr = TA_TPRI | TA_WMUL;			// Set EventFlag Attribute
#if USE_OBJECT_NAME
	u.t_cflg.flgatr |= TA_DSNAME;				// Set EventFlag Attribute
#ifdef CLANGSPEC
	strcpy( u.t_cflg.dsname, "qspi_f" );			// Set Debugger Suport Name
#else
	strcpy( (char*)u.t_cflg.dsname, "qspi_f" );		// Set Debugger Suport Name
#endif /* CLANGSPEC */
#endif /* USE_OBJECT_NAME */
	u.t_cflg.iflgptn = 0;					// Set Initial Bit Pattern
	if( (objid = tk_cre_flg( &u.t_cflg )) <= E_OK )		// Create QSPI EventFlag
		goto ERROR;
	flgid = objid;						// Set QSPI EventFlag ID

	QSPI_Init( );						// Initialize QSPI Controller

	u.t_ddev.exinf = NULL;					// Set Extend Information
	u.t_ddev.drvatr = 0;					// Set Driver Attribute
	u.t_ddev.nsub = 0;					// Set Sub Unit Number
	u.t_ddev.blksz = 1;					// Set Block Size
#ifdef CLANGSPEC
	u.t_ddev.openfn  = qspi_open;				// Set Open function Address
	u.t_ddev.closefn = qspi_close;				// Set Close function Address
	u.t_ddev.execfn  = qspi_exec;				// Set Execute function Address
	u.t_ddev.waitfn  = qspi_wait;				// Set Wait function Address
	u.t_ddev.abortfn = qspi_abort;				// Set Abort function Address
	u.t_ddev.eventfn = qspi_event;				// Set Event function Address
	return tk_def_dev( QSPISF_DEVNM, &u.t_ddev, NULL );	// Define Device Driver
#else
	u.t_ddev.openfn  = (FP)qspi_open;			// Set Open function Address
	u.t_ddev.closefn = (FP)qspi_close;			// Set Close function Address
	u.t_ddev.execfn  = (FP)qspi_exec;			// Set Execute function Address
	u.t_ddev.waitfn  = (FP)qspi_wait;			// Set Wait function Address
	u.t_ddev.abortfn = (FP)qspi_abort;			// Set Abort function Address
	u.t_ddev.eventfn = (FP)qspi_event;			// Set Event function Address
	return tk_def_dev( (UB*)QSPISF_DEVNM, &u.t_ddev, NULL );// Define Device Driver
#endif	/* CLANGSPEC */
ERROR:
	while( 1 )  ;
}
