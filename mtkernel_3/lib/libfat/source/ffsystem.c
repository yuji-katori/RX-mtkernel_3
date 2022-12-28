/*------------------------------------------------------------------------*/
/* Sample Code of OS Dependent Functions for FatFs                        */
/* (C)ChaN, 2018                                                          */
/*------------------------------------------------------------------------*/
/*    Modified by Yuji Katori at 2022/11/22.                              */	
/*------------------------------------------------------------------------*/

#include <string.h>
#include "ff.h"


#if FF_USE_LFN == 3	/* Dynamic memory allocation */
#include "kernel.h"
/*------------------------------------------------------------------------*/
/* Allocate a memory block                                                */
/*------------------------------------------------------------------------*/

void* ff_memalloc (	/* Returns pointer to the allocated memory block (null if not enough core) */
	UINT msize		/* Number of bytes to allocate */
)
{
	return knl_Imalloc(msize);	/* Allocate a new memory block with POSIX API */
}


/*------------------------------------------------------------------------*/
/* Free a memory block                                                    */
/*------------------------------------------------------------------------*/

void ff_memfree (
	void* mblock	/* Pointer to the memory block to free (nothing to do if null) */
)
{
	knl_Ifree(mblock);	/* Free the memory block with POSIX API */
}

#endif



#if FF_FS_REENTRANT	/* Mutal exclusion */

/*------------------------------------------------------------------------*/
/* Create a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to create a new
/  synchronization object for the volume, such as semaphore and mutex.
/  When a 0 is returned, the f_mount() function fails with FR_INT_ERR.
*/

//const osMutexDef_t Mutex[FF_VOLUMES];	/* Table of CMSIS-RTOS mutex */


int ff_cre_syncobj (	/* 1:Function succeeded, 0:Could not create the sync object */
	BYTE vol,			/* Corresponding volume (logical drive number) */
	FF_SYNC_t* sobj		/* Pointer to return the created sync object */
)
{
#if FF_FS_REENTRANT == 1	// Semaphore
T_CSEM t_csem;

	t_csem.exinf = 0;
#if USE_OBJECT_NAME
	t_csem.sematr = TA_TPRI | TA_FIRST | TA_DSNAME;
#ifdef CLANGSPEC
	strcpy( t_csem.dsname, "FatFs" );
#else
	strcpy( (char*)t_csem.dsname, "FatFs" );
#endif	/* CLANGSPEC */
#else
	t_csem.sematr = TA_TPRI | TA_FIRST;
#endif	/* USE_OBJECT_NAME */
	t_csem.isemcnt = 1;
	t_csem.maxsem = 1;
	*sobj = tk_cre_sem( &t_csem );
#else				// Mutex
T_CMTX t_cmtx;

	t_cmtx.exinf = 0;
#if USE_OBJECT_NAME
	t_cmtx.mtxatr = TA_TPRI | TA_DSNAME;
#ifdef CLANGSPEC
	strcpy( t_cmtx.dsname, "FatFs" );
#else
	strcpy( (char*)t_cmtx.dsname, "FatFs" );
#endif	/* CLANGSPEC */
#else
	t_cmtx.mtxatr = TA_TPRI;
#endif	/* USE_OBJECT_NAME */
	*sobj = tk_cre_mtx( &t_cmtx );
#endif
	return *sobj > 0;
}


/*------------------------------------------------------------------------*/
/* Delete a Synchronization Object                                        */
/*------------------------------------------------------------------------*/
/* This function is called in f_mount() function to delete a synchronization
/  object that created with ff_cre_syncobj() function. When a 0 is returned,
/  the f_mount() function fails with FR_INT_ERR.
*/

int ff_del_syncobj (	/* 1:Function succeeded, 0:Could not delete due to an error */
	FF_SYNC_t sobj		/* Sync object tied to the logical drive to be deleted */
)
{
#if FF_FS_REENTRANT == 1	// Semaphore
	return tk_del_sem(sobj) == E_OK;
#else				// Mutex
	return tk_del_mtx(sobj) == E_OK;
#endif
}


/*------------------------------------------------------------------------*/
/* Request Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on entering file functions to lock the volume.
/  When a 0 is returned, the file function fails with FR_TIMEOUT.
*/

int ff_req_grant (	/* 1:Got a grant to access the volume, 0:Could not get a grant */
	FF_SYNC_t sobj	/* Sync object to wait */
)
{
#if FF_FS_REENTRANT == 1	// Semaphore
	return tk_wai_sem(sobj,1,FF_FS_TIMEOUT) == E_OK;
#else				// Mutex
	return tk_loc_mtx(sobj,FF_FS_TIMEOUT) == E_OK;
#endif
}


/*------------------------------------------------------------------------*/
/* Release Grant to Access the Volume                                     */
/*------------------------------------------------------------------------*/
/* This function is called on leaving file functions to unlock the volume.
*/

void ff_rel_grant (
	FF_SYNC_t sobj	/* Sync object to be signaled */
)
{
#if FF_FS_REENTRANT == 1	// Semaphore
	tk_sig_sem(sobj,1);
#else				// Mutex
	tk_unl_mtx(sobj);
#endif
}

#endif

