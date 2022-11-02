/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/
/*
 *----------------------------------------------------------------------
 *    Modifications for micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Support RAMDISK 2022/10/25
 *----------------------------------------------------------------------
*/
#include <tk/tkernel.h>
#include <dev_disk.h>
#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

LOCAL ID phy_device[DEV_TYPE_CNT];

/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DiskInfo dinfo;
	ER ercd;
	SZ asize;

	if( pdrv < DEV_TYPE_CNT && phy_device[pdrv] > 0 )  {
		ercd = tk_srea_dev(phy_device[pdrv], DN_DISKINFO, &dinfo, sizeof(DiskInfo), &asize);
		if( ercd < E_OK )
			return STA_NODISK;
		return dinfo.protect ? STA_PROTECT : 0 ;
	}
	return STA_NOINIT;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	ER ercd;
	SZ asize;

	switch( pdrv )  {
	case RAMDISK:
		if( phy_device[pdrv] <= 0)  {
#ifdef CLANGSPEC
			ercd = tk_opn_dev(RAM_DISK_DEVNM, TD_UPDATE | TD_EXCL);
#else
			ercd = tk_opn_dev((UB*)RAM_DISK_DEVNM, TD_UPDATE | TD_EXCL);
#endif	/* CLANGSPEC */
			if( ercd < E_OK )
				return STA_NOINIT;
			phy_device[pdrv] = ercd;
			tk_swri_dev( ercd, DN_DISKFORMAT, NULL, 0, &asize );
		}
		break;
	}
	return disk_status(pdrv);
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,		/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	ER ercd;
	SZ asize;
	
	if( pdrv >= DEV_TYPE_CNT )
		return RES_PARERR;
	if( phy_device[pdrv] <= 0 )
		return RES_NOTRDY;
	ercd = tk_srea_dev(phy_device[pdrv], sector, buff, count, &asize);
	if( ercd < 0 || asize < count )
		return RES_ERROR;
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count		/* Number of sectors to write */
)
{
	ER ercd;
	SZ asize;
	
	if( pdrv >= DEV_TYPE_CNT )
		return RES_PARERR;
	if( phy_device[pdrv] <= 0 )
		return RES_NOTRDY;
	ercd = tk_swri_dev(phy_device[pdrv], sector, buff, count, &asize);
	if( ercd < 0 || asize < count )
		return RES_ERROR;
	return RES_OK;
}

#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DiskCHSInfo chsinfo;
	DiskInfo dinfo;
	ER ercd;
	SZ asize;
	
	if( pdrv >= DEV_TYPE_CNT )
		return RES_PARERR;
	if( phy_device[pdrv] <= 0 )
		return RES_NOTRDY;
	switch( cmd )  {
	case CTRL_SYNC:				/* Do nothing. */
		return RES_OK;
	case GET_SECTOR_COUNT:
		ercd = tk_srea_dev(phy_device[pdrv], DN_DISKCHSINFO, &chsinfo, sizeof(DiskCHSInfo), &asize);
		if( ercd >= E_OK )  {
			*((DWORD *) buff) = (chsinfo.cylinder * chsinfo.head * chsinfo.sector);
			return RES_OK;
		}
		break;
	case GET_SECTOR_SIZE:
		ercd = tk_srea_dev(phy_device[pdrv], DN_DISKCHSINFO, &chsinfo, sizeof(DiskCHSInfo), &asize);
		if( ercd >= E_OK )  {
			ercd = tk_srea_dev(phy_device[pdrv], DN_DISKINFO, &dinfo, sizeof(DiskInfo), &asize);
			if( ercd >= E_OK )  {
				*((DWORD *) buff) = dinfo.blockcont / (chsinfo.cylinder * chsinfo.head * chsinfo.sector) * dinfo.blocksize;
				return RES_OK;
			}
		}
		break;
	case GET_BLOCK_SIZE:
		ercd = tk_srea_dev(phy_device[pdrv], DN_DISKINFO, &dinfo, sizeof(DiskInfo), &asize);
		if( ercd >= E_OK )  {
			*((DWORD *) buff) = dinfo.blocksize;
			return RES_OK;
		}
		break;
	default:
		return RES_PARERR;
	}
	return RES_ERROR;
}