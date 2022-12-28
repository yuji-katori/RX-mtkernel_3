/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2022/11/17.
 *----------------------------------------------------------------------
 */

/*
 *	rd_main.c
 *
 *	RAM Disk Driver
 */

#include <string.h>
#include <tk/tkernel.h>
#include <dev_rd.h>

LOCAL ER rd_open(ID devid, UINT omode, void *exinf)
{
	return E_OK;
}

LOCAL ER rd_close(ID devid, UINT option, void *exinf)
{
	return E_OK;
}

LOCAL ER rd_exec(T_DEVREQ *devreq, TMO tmout, void *exinf)
{
void *buf = devreq->buf;
W   start = devreq->start;
SZ  size  = devreq->size;
INT *p;
	devreq->asize = size;
	devreq->error = E_OK;
	if( devreq->cmd == TDC_READ )  {
		switch( start )  {
		case DN_DISKEVENT:
		case DN_DISKPARTINFO:
		case DN_DISKIDINFO:
			return E_NOSPT;
		case DN_DISKMEMADR:
			if( size == sizeof(void *) && buf != NULL )  {
				*(size_t *)buf = TOP_ADDRESS;
				return E_OK;
			}
			break;
		case DN_DISKCHSINFO:
			if( size == sizeof(DiskCHSInfo) && buf != NULL )  {
				DiskCHSInfo *di = buf;
				di->cylinder = 1;
				di->head     = 1;
				di->sector   = BLOCK_COUNT;
				return E_OK;
			}
			break;
		case DN_DISKINFO:
			if( size == sizeof(DiskInfo) && buf != NULL )  {
				DiskInfo *di = buf;
				di->format  = DiskFmt_MEM;
				di->protect = 0;
				di->removable = 0;
				di->blocksize = BLOCK_SIZE;
				di->blockcont = BLOCK_COUNT;
				return E_OK;
			}
			break;
		default:
			if( start >= 0 && buf != NULL && start + size <= BLOCK_COUNT )  {
				memcpy( buf, (void*)(TOP_ADDRESS + start * BLOCK_SIZE), size * BLOCK_SIZE );
				return E_OK;
			}
		}
	}
	else  {
		switch( start ) {
		case DN_DISKEVENT:
		case DN_DISKINIT:
		case DN_DISKCMD:
			return E_NOSPT;
		case DN_DISKFORMAT:
			if( size == sizeof(DiskFormat) && buf != NULL && *(DiskFormat*)buf == DiskFmt_MEM )  {
				for( p=(INT*)TOP_ADDRESS; p!=(INT*)END_ADDRESS ; p++ )
					*p = -1;
				return E_OK;
			}
			break;
		default:
			if( start >= 0 && buf != NULL && start + size <= BLOCK_COUNT )  {
				memcpy( (void*)(TOP_ADDRESS + start * BLOCK_SIZE), buf, size * BLOCK_SIZE );
				return E_OK;
			}
		}
	}
	return E_PAR;
}

LOCAL INT rd_wait(T_DEVREQ *devreq, INT nreq, TMO tmout, void *exinf)
{
	return 0;
}

LOCAL ER rd_abort(ID tskid, T_DEVREQ *devreq, INT nreq, void *exinf)
{
	return E_OK;
}

LOCAL INT rd_event(INT evttyp, void *evtinf, void *exinf)
{
	return E_OK;
}

EXPORT ER rdDrvEntry(void)
{
T_DDEV t_ddev;

	t_ddev.exinf = NULL;				// Set Extend Information
	t_ddev.drvatr = TDK_DISK_RAM;			// Set Driver Attribute
	t_ddev.nsub = 0;				// Set Sub Unit Number
	t_ddev.blksz = BLOCK_SIZE;			// Set Block Size
#ifdef CLANGSPEC
	t_ddev.openfn  = rd_open;			// Set Open function Address
	t_ddev.closefn = rd_close;			// Set Close function Address
	t_ddev.execfn  = rd_exec;			// Set Execute function Address
	t_ddev.waitfn  = rd_wait;			// Set Wait function Address
	t_ddev.abortfn = rd_abort;			// Set Abort function Address
	t_ddev.eventfn = rd_event;			// Set Event function Address
	return tk_def_dev( RAM_DISK_DEVNM, &t_ddev, NULL );
#else
	t_ddev.openfn  = (FP)rd_open;			// Set Open function Address
	t_ddev.closefn = (FP)rd_close;			// Set Close function Address
	t_ddev.execfn  = (FP)rd_exec;			// Set Execute function Address
	t_ddev.waitfn  = (FP)rd_wait;			// Set Wait function Address
	t_ddev.abortfn = (FP)rd_abort;			// Set Abort function Address
	t_ddev.eventfn = (FP)rd_event;			// Set Event function Address
	return tk_def_dev( (UB*)RAM_DISK_DEVNM, &t_ddev, NULL );
#endif	/* CLANGSPEC */
}