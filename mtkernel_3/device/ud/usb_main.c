/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	usb_main.c
 *
 *	USB Disk Driver
 */

#include <string.h>
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <dev_ud.h>

typedef enum { TSKID, FLGID, OBJ_KIND_NUM } OBJ_KIND;
LOCAL ID ObjID[OBJ_KIND_NUM];
LOCAL UB rd, wt;
LOCAL T_DEVREQ *req[CFN_MAX_REQDEV+1];
LOCAL UINT now, next=MAXIMUM;
#if !USE_IMALLOC
LOCAL INT usb_task_stack[320/sizeof(INT)];
#endif /* USE_IMALLOC */

LOCAL ER ud_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER ud_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER ud_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
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

LOCAL INT ud_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
UINT flgptn;
ER ercd;
	ercd = tk_wai_flg( ObjID[FLGID], (UINT)devreq->exinf, TWF_ORW | TWF_BITCLR, &flgptn, tmout );
	devreq->exinf = NULL;							// Clear Flag Pattern
	return ercd;								// Return
}

LOCAL ER ud_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
	tk_dis_dsp( );								// Disable Dispatch
	now &= ~((UINT)devreq->exinf);						// Clear Flag Pattern
	tk_set_flg( ObjID[FLGID], (UINT)devreq->exinf );			// Wakeup Request Task
	devreq->exinf = NULL;							// Clear Flag Pattern
	devreq->error = E_ABORT;						// Set Error Code
	tk_ena_dsp( );								// Enable Dispatch
	return E_OK;
}

LOCAL INT ud_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

LOCAL ER USB_Read(T_DEVREQ *devreq)
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
			if( USB_GetStatus( ) == USB_NO_MEM )
				return E_NOMDA;
			di->cylinder = 1;
			di->head     = 1;
			di->sector   = USB_GetBlockCount( );
			return E_OK;
		}
		break;
	case DN_DISKINFO:
		if( size == sizeof(DiskInfo) && buf != NULL )  {
			DiskInfo *di = buf;
			if( USB_GetStatus( ) == USB_NO_MEM )
				return E_NOMDA;
			di->format  = DiskFmt_STD;
			di->protect = 0;
			di->removable = 0;
			di->blocksize = BLOCK_SIZE;
			di->blockcont = USB_GetBlockCount( );
			return E_OK;
		}
		break;
	default:
		if( start >= 0 && buf != NULL && size )
			return USB_ReadBlock(buf, start, size);
	}
	return E_PAR;
}

LOCAL ER USB_Write(T_DEVREQ *devreq)
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
			return USB_WriteBlock(buf, start, size);
	}
	return E_PAR;
}

LOCAL void usb_tsk(INT stacd, void *exinf)
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
				devreq->error = USB_Read( devreq );		// USB Memory Read
			else							// Write Command
				devreq->error = USB_Write( devreq );		// USB Memory Write
			tk_dis_dsp( );						// Disable Dispatch
			now &= ~((UINT)devreq->exinf);				// Clear Flag Pattern
			tk_set_flg( ObjID[FLGID], (UINT)devreq->exinf );	// Wakeup Request Task
			tk_ena_dsp( );						// Enable Dispatch
		}
		if( flgptn & USBEVENT )  {					// USB Event ?
			USB_Schedule( );					// USB Event Schedule
		}
		if( flgptn & USBDETACH )  {					// USB Detach ?
			tm_putstring("Detach USB Memory.\n");
			tk_clr_flg( ObjID[FLGID], ~ATTACH );			// Clear Attach Flag
			tk_set_flg( ObjID[FLGID],  DETACH );			// Set   Detach Flag
		}
		if( flgptn & USBATTACH )  {					// USB Attach ?
			tm_putstring("Attach USB Memory.\n");
			USB_GetCapacity( );					// Get USB Capacity
		}
		if( flgptn & USBCAPACITY )  {					// USB Get Capacity ?
			tk_clr_flg( ObjID[FLGID], ~DETACH );			// Clear Detach Flag
			tk_set_flg( ObjID[FLGID],  ATTACH );			// Set   Attach Flag
		}
	}
}

EXPORT ER usbDrvEntry(void)
{
ID objid;
union { T_CTSK t_ctsk; T_CFLG t_cflg; T_DDEV t_ddev; T_DINT t_dint; } u;

	u.t_ctsk.tskatr  = TA_HLNG;			// Set Task Attribute
#if USE_OBJECT_NAME
	u.t_ctsk.tskatr |= TA_DSNAME;			// Set Task Attribute
#endif /* USE_OBJECT_NAME */
#if !USE_IMALLOC
	u.t_ctsk.tskatr |= TA_USERBUF;			// Set Task Attribute
	u.t_ctsk.bufptr = usb_task_stack;		// Set Stack Top Address
#endif /* USE_OBJECT_NAME */
	u.t_ctsk.stksz = 2048;				// Set Task StackSize
	u.t_ctsk.itskpri = USB_GetTaskPri( );		// Set Task Priority
#ifdef CLANGSPEC
	u.t_ctsk.task =  usb_tsk;			// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( u.t_ctsk.dsname, "usb_t" );		// Set Task Debugger Suport Name
#endif /* USE_OBJECT_NAME */
#else
	u.t_ctsk.task =  (FP)usb_tsk;			// Set Task Start Address
#if USE_OBJECT_NAME
	strcpy( (char*)u.t_ctsk.dsname, "usb_t" );	// Set Task Debugger Suport Name
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
	strcpy( u.t_cflg.dsname, "usb_f" );		// Set Debugger Suport Name
#else
	strcpy( (char*)u.t_cflg.dsname, "usb_f" );	// Set Debugger Suport Name
#endif /* CLANGSPEC */
#endif /* USE_OBJECT_NAME */
	u.t_cflg.iflgptn = 0;				// Set Initial Bit Pattern
	if( (objid = tk_cre_flg( &u.t_cflg )) <= E_OK )	// Create USB EventFlag
		goto ERROR;
	ObjID[FLGID] = objid;				// Set USB EventFlag ID

	if( USB_Init( objid, &u.t_dint ) < E_OK )	// Initialize USB Host
		goto ERROR;

	u.t_ddev.exinf = NULL;				// Set Extend Information
	u.t_ddev.drvatr = 0;				// Set Driver Attribute
	u.t_ddev.nsub = 0;				// Set Sub Unit Number
	u.t_ddev.blksz = BLOCK_SIZE;			// Set Block Size
#ifdef CLANGSPEC
	u.t_ddev.openfn  = ud_open;			// Set Open function Address
	u.t_ddev.closefn = ud_close;			// Set Close function Address
	u.t_ddev.execfn  = ud_exec;			// Set Execute function Address
	u.t_ddev.waitfn  = ud_wait;			// Set Wait function Address
	u.t_ddev.abortfn = ud_abort;			// Set Abort function Address
	u.t_ddev.eventfn = ud_event;			// Set Event function Address
	return tk_def_dev( USB_MSC_DEVNM, &u.t_ddev, NULL );
#else
	u.t_ddev.openfn  = (FP)ud_open;			// Set Open function Address
	u.t_ddev.closefn = (FP)ud_close;		// Set Close function Address
	u.t_ddev.execfn  = (FP)ud_exec;			// Set Execute function Address
	u.t_ddev.waitfn  = (FP)ud_wait;			// Set Wait function Address
	u.t_ddev.abortfn = (FP)ud_abort;		// Set Abort function Address
	u.t_ddev.eventfn = (FP)ud_event;		// Set Event function Address
	return tk_def_dev( (UB*)USB_HMSC_DEVNM, &u.t_ddev, NULL );
#endif	/* CLANGSPEC */
ERROR:
	while( 1 )  ;		
}

EXPORT ER usbWaitAttachEvent(TMO tmout)
{
UINT flgptn;
	return tk_wai_flg(ObjID[FLGID], ATTACH, TWF_ORW, &flgptn, tmout);
}

EXPORT ER usbWaitDetachEvent(TMO tmout)
{
UINT flgptn;
	return tk_wai_flg(ObjID[FLGID], DETACH, TWF_ORW, &flgptn, tmout);
}