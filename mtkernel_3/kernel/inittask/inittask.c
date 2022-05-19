/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2019 by Ken Sakamura.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 */

/*
 *	inittask.c
 *	Initial task definition
 */
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include <sys/inittask.h>

#include "kernel.h"

#if !USE_IMALLOC
INT	init_task_stack[INITTASK_STKSZ/sizeof(INT)];
#endif

typedef INT	(*MAIN_FP)(INT, UB **);

/*
 * Initial task creation parameter
 */

#ifdef CLANGSPEC
EXPORT void init_task_main( INT stacd , void * exinf );
#else
EXPORT void init_task_main(void);
#endif /* CLANGSPEC */

EXPORT const T_CTSK knl_init_ctsk = {
	(void *)INITTASK_EXINF,		/* exinf */
	INITTASK_TSKATR,		/* tskatr */
#ifdef CLANGSPEC
	init_task_main,			/* task */
#else
	(FP)&init_task_main,		/* task */
#endif /* CLANGSPEC */
	INITTASK_ITSKPRI,		/* itskpri */
	INITTASK_STKSZ,			/* stksz */
#if USE_OBJECT_NAME
	INITTASK_DSNAME,		/* dsname */
#endif
	INITTASK_STACK,			/* bufptr */
};

/* --------------------------------------------------------------- */
/*
 * Start System
 *	Start each subsystem and each device driver.
 *	Return from function after starting.
 */
LOCAL ER start_system( void )
{
	ER	ercd;

#if USE_DEVICE
	/* Initialize Device manager */
	ercd = knl_initialize_devmgr();
	if ( ercd < E_OK ) return ercd;
#endif

	/* Start system dependent sequence */
	ercd = knl_start_device();

	return ercd;
}

/*
 * Stop System
 *	Never return from this function.
 *
 *	fin  =	 0 : Power off
 *		-1 : reset and re-start	(Reset -> Boot -> Start)
 *		-2 : fast re-start		(Start)
 *		-3 : Normal re-start		(Boot -> Start)
 *
 *	fin are not always supported.
 */
LOCAL void shutdown_system( INT fin )
{
#if USE_SHUTDOWN
	/* Platform dependent finalize sequence */
	knl_finish_device();

	/* Shutdown message output */
	if ( fin >= 0 ) {
		SYSTEM_MESSAGE("\n<< SYSTEM SHUTDOWN >>\n");
	}

	if ( fin < 0 ) {
		/* Re-start sequence (platform dependent) */
		knl_restart_device(fin);
	}

	knl_tkernel_exit();		/* Stop system */
#else
	DISABLE_INTERRUPT;
	for(;;) {
		;
	}
#endif /* USE_SHUTDOWN */
}


/*
 * Initial task main
 */
#ifdef CLANGSPEC
EXPORT void init_task_main( INT stacd , void * exinf )
#else
EXPORT void init_task_main(void)
#endif /* CLANGSPEC */
{
	INT	fin = 1;
	ER	ercd;

	ercd = start_system();		/* Start Sub-system & device driver */
	if(ercd  >= E_OK) {

#if (USE_SYSTEM_MESSAGE && USE_TMONITOR)
#ifdef CLANGSPEC
		tm_printf("\n\nmicroT-Kernel Version %x.%02x\n\n", VER_MAJOR, VER_MINOR);
#else
		tm_printf((UB*)"\n\nmicroT-Kernel Version %x.%02x\n\n", VER_MAJOR, VER_MINOR);
#endif /* CLANGSPEC */
#endif

#if USE_USERINIT
		/* Perform user defined initialization sequence */
		fin = (*(MAIN_FP)RI_USERINIT)(0, NULL);
#endif
		if ( fin > 0 ) {
			fin = usermain();	/* User Main Program */
		}
#if USE_USERINIT
		/* Perform user defined finalization sequence */
		(*(MAIN_FP)RI_USERINIT)(-1, NULL);
#endif

	} else {
		SYSTEM_MESSAGE("!ERROR! Init Task start\n");	/* Start message */
	}

	shutdown_system(fin);	/* Never return */
}

