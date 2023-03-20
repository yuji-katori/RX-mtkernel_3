/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2023/1/19.
 *----------------------------------------------------------------------
 */

/*
 *	sd_main.c
 *
 *	SD Card Driver
 */

#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_sd.h>

typedef enum { TSKID, FLGID, OBJ_KIND_NUM } OBJ_KIND;
LOCAL ID ObjID[OBJ_KIND_NUM];
LOCAL UB rd, wt;
LOCAL T_DEVREQ *req[CFN_MAX_REQDEV+1];
LOCAL UINT now, next=MAXIMUM;
#if !USE_IMALLOC
LOCAL INT sdc_task_stack[320/sizeof(INT)];
#endif /* USE_IMALLOC */

LOCAL ER sd_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER sd_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER sd_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
ER ercd;
	tk_dis_dsp( );								// Disable Dispatch
	if( now & next )							// Check Request Count
		ercd = E_LIMIT;							// Over Request Count
	else  {
		now |= (UINT)devreq->exinf = next;				// Set Flag Pattern
		if( ( next >>= 1 ) == MINIMUM )					// Minimum Value ?
			next = MAXIMUM;						// Set Maximum Value
		req[wt] = devreq;						// Set New Device Request
		if( ++wt == CFN_MAX_REQDEV+1 )					// Write Pointer is Max ?
			wt = 0;							// Clear Write Pointer
		tk_set_flg( ObjID[FLGID], EXECCMD );				// Wakeup sdc_tsk
		ercd = E_OK;							// Normal return
	}
	tk_ena_dsp( );								// Enable Dispatch
	return ercd;
}

LOCAL INT sd_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
UINT flgptn;
ER ercd;
	ercd = tk_wai_flg( ObjID[FLGID], (UINT)devreq->exinf, TWF_ORW | TWF_BITCLR, &flgptn, tmout );
	devreq->exinf = NULL;							// Clear Flag Pattern
	return ercd;								// Return
}

LOCAL ER sd_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
	tk_dis_dsp( );								// Disable Dispatch
	now &= ~((UINT)devreq->exinf);						// Clear Flag Pattern
	tk_set_flg( ObjID[FLGID], (UINT)devreq->exinf );			// Wakeup Request Task
	devreq->exinf = NULL;							// Clear Flag Pattern
	devreq->error = E_ABORT;						// Set Error Code
	tk_ena_dsp( );								// Enable Dispatch
	return E_OK;
}

LOCAL INT sd_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

LOCAL ER SDC_Read(T_DEVREQ *devreq)
{
void *buf = devreq->buf;
W   start = devreq->start;
SZ  size  = devreq->size;
	devreq->asize = size;
	switch( start )  {
	case DN_DISKEVENT:
	case DN_DISKMEMADR:
	case DN_DISKPARTINFO:
	case DN_DISKIDINFO:
		return E_NOSPT;
	case DN_DISKCHSINFO:
		if( size == sizeof(DiskCHSInfo) && buf != NULL )  {
			DiskCHSInfo *di = buf;
			if( SDC_GetStatus( ) == SD_NO_CARD )
				return E_NOMDA;
			di->cylinder = 1;
			di->head     = 1;
			di->sector   = SDC_GetBlockCount( );
			return E_OK;
		}
		break;
	case DN_DISKINFO:
		if( size == sizeof(DiskInfo) && buf != NULL )  {
			DiskInfo *di = buf;
			if( SDC_GetStatus( ) == SD_NO_CARD )
				return E_NOMDA;
			di->format  = DiskFmt_STD;
			di->protect = SDC_GetStatus( ) == SD_RO_CARD ? 1 : 0;
			di->removable = 0;
			di->blocksize = BLOCK_SIZE;
			di->blockcont = SDC_GetBlockCount( );
			return E_OK;
		}
		break;
	default:
		if( start >= 0 && buf != NULL && size )
			return SDC_ReadBlock(buf, start, size);
	}
	return E_PAR;
}

LOCAL ER SDC_Write(T_DEVREQ *devreq)
{
void *buf = devreq->buf;
W   start = devreq->start;
SZ  size  = devreq->size;
	devreq->asize = size;
	switch( start )  {
	case DN_DISKEVENT:
	case DN_DISKFORMAT:
	case DN_DISKINIT:
	case DN_DISKCMD:
		return E_NOSPT;
	default:
		if( start >= 0 && buf != NULL && size )
			return SDC_WriteBlock(buf, start, size);
	}
	return E_PAR;
}

LOCAL void sdc_tsk(INT stacd, void *exinf)
{
UINT flgptn;
T_DEVREQ *devreq;
	while( 1 )  {
		tk_wai_flg( ObjID[FLGID], TSK_WAIT_ALL, TWF_ORW | TWF_BITCLR, &flgptn, TMO_FEVR );
		if( flgptn & EXECCMD )  {					// Execute Commond ?
			tk_dis_dsp( );						// Disable Dispatch
			devreq = req[rd];					// Read Device Request
			if( ++rd == CFN_MAX_REQDEV+1 )				// Read Pointer is Max ?
				rd = 0;						// Clear Read Pointer
			if( wt != rd )						// Nothing Device Request ?
				tk_set_flg( ObjID[FLGID], EXECCMD );		// Set Execute Command Bit
			tk_ena_dsp( );						// Enable Dispatch
			if( devreq->abort )					// Abort request ?
				devreq->error = E_ABORT;			// Set Error Code
			else if( devreq->cmd == TDC_READ )			// Command is Read ?
				devreq->error = SDC_Read( devreq );		// SD Card Read
			else							// Write Command
				devreq->error = SDC_Write( devreq );		// SD Card Write
			tk_dis_dsp( );						// Disable Dispatch
			now &= ~((UINT)devreq->exinf);				// Clear Flag Pattern
			tk_set_flg( ObjID[FLGID], (UINT)devreq->exinf );	// Wakeup Request Task
			tk_ena_dsp( );						// Enable Dispatch
		}
		if( flgptn & CARD_REJECT )  {					// SD Card Reject ?
			tm_putstring("Reject SD Card.\n");
			tk_clr_flg( ObjID[FLGID], ~INSERT );			// Clear Insert Flag
			tk_set_flg( ObjID[FLGID], REJECT );			// Set   Reject Flag
			SDC_CardReject( );					// Call SD Card Reject
		}
		if( flgptn & CARD_INSERT )  {					// SD Card Insert ?
			tm_putstring("Insert SD Card.\n");
			tk_clr_flg( ObjID[FLGID], ~REJECT );			// Clear Reject Flag
			tk_set_flg( ObjID[FLGID], INSERT );			// Set   Insert Flag
			if( SDC_CardInsert( ) >= E_OK )				// Call SD Card Insert
				if( SDC_InitCard( ) < E_OK )			// Call SD Card Initialize
					tm_putstring("Insert SD Card is Unknow Card\n");
		}
	}
}

EXPORT ER sdDrvEntry(void)
{
ID objid;
union { T_CTSK t_ctsk; T_CFLG t_cflg; T_DDEV t_ddev; T_DINT t_dint; } u;

	u.t_ctsk.tskatr  = TA_HLNG;			// Set Task Attribute
#if USE_OBJECT_NAME
	u.t_ctsk.tskatr |= TA_DSNAME;			// Set Task Attribute
#endif /* USE_OBJECT_NAME */
#if !USE_IMALLOC
	u.t_ctsk.tskatr |= TA_USERBUF;			// Set Task Attribute
	u.t_ctsk.bufptr = sdc_task_stack;		// Set Stack Top Address
#endif /* USE_OBJECT_NAME */
	u.t_ctsk.stksz = 320;				// Set Task StackSize
	u.t_ctsk.itskpri = SDC_GetTaskPri( );		// Set Task Priority
#ifdef CLANGSPEC
	u.t_ctsk.task =  sdc_tsk;			// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( u.t_ctsk.dsname, "sdc_t" );		// Set Task Debugger Suport Name
#endif /* USE_OBJECT_NAME */
#else
	u.t_ctsk.task =  (FP)sdc_tsk;			// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)u.t_ctsk.dsname, "sdc_t" );	// Set Task Debugger Suport Name
#endif /* USE_OBJECT_NAME */
#endif /* CLANGSPEC */
	if( (objid = tk_cre_tsk( &u.t_ctsk )) <= E_OK )	// Create SD Control Task
		goto ERROR;
	if( tk_sta_tsk( objid, 0 ) < E_OK )		// Start SD Control Task
		goto ERROR;
	ObjID[TSKID] = objid;				// Set SD Control Task ID

	u.t_cflg.flgatr = TA_TPRI | TA_WMUL;		// Set EventFlag Attribute
#if USE_OBJECT_NAME
	u.t_cflg.flgatr |= TA_DSNAME;			// Set EventFlag Attribute
#ifdef CLANGSPEC
	strcpy( u.t_cflg.dsname, "sdc_f" );		// Set Debugger Suport Name
#else
	strcpy( (char*)u.t_cflg.dsname, "sdc_f" );	// Set Debugger Suport Name
#endif /* CLANGSPEC */
#endif /* USE_OBJECT_NAME */
	u.t_cflg.iflgptn = 0;				// Set Initial Bit Pattern
	if( (objid = tk_cre_flg( &u.t_cflg )) <= E_OK )	// Create SD EventFlag
		goto ERROR;
	ObjID[FLGID] = objid;				// Set SD EventFlag ID

	if( SDC_Init( objid, &u.t_dint ) < E_OK )	// Initialize SD Card Controller
		goto ERROR;

	u.t_ddev.exinf = NULL;				// Set Extend Information
	u.t_ddev.drvatr = 0;				// Set Driver Attribute
	u.t_ddev.nsub = 0;				// Set Sub Unit Number
	u.t_ddev.blksz = BLOCK_SIZE;			// Set Block Size
#ifdef CLANGSPEC
	u.t_ddev.openfn  = sd_open;			// Set Open function Address
	u.t_ddev.closefn = sd_close;			// Set Close function Address
	u.t_ddev.execfn  = sd_exec;			// Set Execute function Address
	u.t_ddev.waitfn  = sd_wait;			// Set Wait function Address
	u.t_ddev.abortfn = sd_abort;			// Set Abort function Address
	u.t_ddev.eventfn = sd_event;			// Set Event function Address
	return tk_def_dev( SD_CARD_DEVNM, &u.t_ddev, NULL );
#else
	u.t_ddev.openfn  = (FP)sd_open;			// Set Open function Address
	u.t_ddev.closefn = (FP)sd_close;		// Set Close function Address
	u.t_ddev.execfn  = (FP)sd_exec;			// Set Execute function Address
	u.t_ddev.waitfn  = (FP)sd_wait;			// Set Wait function Address
	u.t_ddev.abortfn = (FP)sd_abort;		// Set Abort function Address
	u.t_ddev.eventfn = (FP)sd_event;		// Set Event function Address
	return tk_def_dev( (UB*)SD_CARD_DEVNM, &u.t_ddev, NULL );
#endif	/* CLANGSPEC */
ERROR:
	while( 1 )  ;		
}

IMPORT ER sdWaitInsertEvent(TMO tmout)
{
UINT flgptn;
	return tk_wai_flg(ObjID[FLGID], INSERT, TWF_ORW, &flgptn, tmout);
}

IMPORT ER sdWaitRejectEvent(TMO tmout)
{
UINT flgptn;
	return tk_wai_flg(ObjID[FLGID], REJECT, TWF_ORW, &flgptn, tmout);
}