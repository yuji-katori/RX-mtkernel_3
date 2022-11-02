/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	rd_main.c
 *
 *	RAM Disk Driver
 */

#include <string.h>
#include <stdlib.h>
#include <tk/tkernel.h>
#include <driver/sdrvif.h>
#include <dev_disk.h>

#define	TOP_ADDRESS	0x8000000
#define	END_ADDRESS	0xA000000
#define	BLOCK_SIZE	512

LOCAL ER rd_readfn(ID devid, W start, SZ size, void *buf, void *exinf)
{
	switch( start )  {
	case DN_DISKEVENT:
	case DN_DISKPARTINFO:
	case DN_DISKIDINFO:
		return E_NOSPT;
	case DN_DISKMEMADR:
		if( size == sizeof(void *) && buf != NULL )  {
			*(size_t *)buf = TOP_ADDRESS;
			return size;
		}
		break;
	case DN_DISKCHSINFO:
		if( size == sizeof(DiskCHSInfo) && buf != NULL )  {
			DiskCHSInfo *di = buf;
			di->cylinder = 1;
			di->head     = 1;
			di->sector   = ( END_ADDRESS - TOP_ADDRESS ) / BLOCK_SIZE;
			return size;
		}
		break;
	case DN_DISKINFO:
		if( size == sizeof(DiskInfo) && buf != NULL )  {
			DiskInfo *di = buf;
			di->format = DiskFmt_MEM;
			di->protect = 0;
			di->removable = 0;
			di->blocksize = BLOCK_SIZE;
			di->blockcont = ( END_ADDRESS - TOP_ADDRESS ) / BLOCK_SIZE;
			return size;
		}
		break;
	default:
		if( start >= 0 )  {
			if( size )  {
				memcpy( buf, (void*)(TOP_ADDRESS + start * BLOCK_SIZE), size * BLOCK_SIZE );
				return size;
			}
			else
				return END_ADDRESS - TOP_ADDRESS;
		}
	}
	return E_PAR;
}

LOCAL ER rd_writefn(ID devid, W start, SZ size, void *buf, void *exinf)
{
INT *p;
	switch( start ) {
	case DN_DISKEVENT:
	case DN_DISKINIT:
	case DN_DISKCMD:
		return E_NOSPT;
	case DN_DISKFORMAT:
		for( p=(INT*)TOP_ADDRESS; p!=(INT*)END_ADDRESS ; p++ )
			*p = -1;
		return END_ADDRESS - TOP_ADDRESS;
	default:
		if( start >= 0 ){
			memcpy( (void*)(TOP_ADDRESS + start * BLOCK_SIZE), buf, size * BLOCK_SIZE );
			return size;
		}
	}
	return E_PAR;
}

LOCAL SDI     sdi;

EXPORT ER rdDrvEntry(void)
{
SDefDev ddev;
T_IDEV  idev;

	/* Set parameters for SDefDevice. */
	ddev.exinf  = NULL;
#ifdef CLANGSPEC
	strcpy( ddev.devnm, RAM_DISK_DEVNM );
#else
	strcpy( (char*)ddev.devnm, RAM_DISK_DEVNM );
#endif	/* CLANGSPEC */
	ddev.drvatr = 0;
	ddev.devatr = 0;
	ddev.nsub   = 0;
	ddev.blksz  = BLOCK_SIZE;
	ddev.open   = NULL;
	ddev.close  = NULL;
	ddev.read   = rd_readfn;
	ddev.write  = rd_writefn;
	ddev.event  = NULL;
	/* Register this device driver. */
	return SDefDevice( &ddev, &idev, &sdi );
}