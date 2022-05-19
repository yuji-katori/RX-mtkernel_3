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
 *	kernel.h
 *	micro T-Kernel Common Definition
 */

#ifndef _KERNEL_
#define _KERNEL_

#include <sys/machine.h>
#include <sys/queue.h>

#include <tk/typedef.h>
#include <tk/errno.h>
#include <tk/syscall.h>
#include <tk/dbgspt.h>

#include "tstdlib.h"

typedef struct task_control_block	TCB;

#include "../tkernel/timer.h"
#include "../tkernel/winfo.h"
#include "../tkernel/mutex.h"

#include "../sysdepend/sys_msg.h"
#include "../sysdepend/cpu_status.h"

#define SYSCALL		EXPORT		/* Definition of system call */

/* User defined handler ( Sub-system calls, time-event handler ) */
# define CallUserHandlerP1(   p1,         hdr, cb)	(*(hdr))(p1)
# define CallUserHandlerP2(   p1, p2,     hdr, cb)	(*(hdr))(p1, p2)
# define CallUserHandlerP3(   p1, p2, p3, hdr, cb)	(*(hdr))(p1, p2, p3)

/*
 * Task control block (TCB)
 */
struct task_control_block {
	QUEUE	tskque;		/* Task queue */
	CTXB	tskctxb;	/* Task context block */
	ID	tskid;		/* Task ID */
	void	*exinf;		/* Extended information */
	ATR	tskatr;		/* Task attribute */
#ifdef CLANGSPEC
	TSKFP	task;		/* Task startup address */
#else
	FP	task;		/* Task startup address */
#endif /* CLANGSPEC */

	SZ	sstksz;		/* stack size */

	INT	:0;		/* ### From here */
	B	isysmode;	/* Task operation mode initial value */
	H	sysmode;	/* Task operation mode, quasi task part
				   call level */
	INT	:0;		/* ### To here, since it might be accessed
				   from outside of the critical section,
				   need to be assigned as an independent
				   word. Also, there is a case where one
				   word is read from 'reqdct' and is read
				   all at once from 'reqdct', 'isysmode',
				   and 'sysmode', so do not change the
				   order and size. */

	UB	ipriority;	/* Priority at task startup */
	UB	bpriority;	/* Base priority */
	UB	priority;	/* Current priority */

	UB /*TSTAT*/	state;	/* Task state (Int. expression) */

	BOOL	klockwait:1;	/* TRUE at wait kernel lock */
	BOOL	klocked:1;	/* TRUE at hold kernel lock */

	CONST WSPEC *wspec;	/* Wait specification */
	ID	wid;		/* Wait object ID */
	INT	wupcnt;		/* Number of wakeup requests queuing */
	INT	suscnt;		/* Number of SUSPEND request nests */
	ER	*wercd;		/* Wait error code set area */
	WINFO	winfo;		/* Wait information */
	TMEB	wtmeb;		/* Wait timer event block */

#if USE_MUTEX
	MTXCB	*mtxlist;	/* List of hold mutexes */
#endif

#if USE_DBGSPT && defined(USE_FUNC_TD_INF_TSK)
	UW	stime;		/* System execution time (ms) */
	UW	utime;		/* User execution time (ms) */
#endif

	void	*isstack;	/* stack pointer initial value */
#if USE_OBJECT_NAME
#ifdef CLANGSPEC
	VB	name[OBJECT_NAME_LENGTH];	/* name */
#else
	UB	name[OBJECT_NAME_LENGTH];	/* name */
#endif /* CLANGSPEC */
#endif
};


/*
 * Task dispatch disable state
 *	0 = DDS_ENABLE		 : ENABLE
 *	1 = DDS_DISABLE_IMPLICIT : DISABLE with implicit process
 *	2 = DDS_DISABLE		 : DISABLE with tk_dis_dsp()
 *	|	|
 *	|	use in *.c
 *	use in *.S
 *	  --> Do NOT change these literals, because using in assembler code
 *
 *	'dispatch_disabled' records dispatch disable status set by tk_dis_dsp()
 *	for some CPU, that accepts delayed interrupt.
 *	In this case, you can NOT refer the dispatch disabled status
 *	only by 'dispatch_disabled'.
 *	Use 'in_ddsp()' to refer the task dispatch status.
 *	'in_ddsp()' is a macro definition in CPU-dependent definition files.
 */
#define DDS_ENABLE		(0)
#define DDS_DISABLE_IMPLICIT	(1)	/* set with implicit process */
#define DDS_DISABLE		(2)	/* set by tk_dis_dsp() */
IMPORT INT	knl_dispatch_disabled;

/*
 * Task in execution
 *	ctxtsk is a variable that indicates TCB task in execution
 *	(= the task that CPU holds context). During system call processing,
 *	when checking information about the task that requested system call,
 *	use 'ctxtsk'. Only task dispatcher changes 'ctxtsk'.
 */
IMPORT TCB	*knl_ctxtsk;

/*
 * Task which should be executed
 *	'schedtsk' is a variable that indicates the task TCB to be executed.
 *	If a dispatch is delayed by the delayed dispatch or dispatch disable, 
 *	it does not match with 'ctxtsk.' 
 */
IMPORT TCB	*knl_schedtsk;


/*
 * Startup / Re-start / Shutdown Hardware (start_dev.c)
 */
IMPORT void knl_startup_device(void);
IMPORT void knl_shutdown_device( void );
IMPORT ER knl_restart_device( INT mode );

/*
 * Kernel-object initialization (each object)
 */
IMPORT ER knl_task_initialize( void );
IMPORT ER knl_semaphore_initialize( void );
IMPORT ER knl_eventflag_initialize( void );
IMPORT ER knl_mailbox_initialize( void );
IMPORT ER knl_messagebuffer_initialize( void );
IMPORT ER knl_rendezvous_initialize( void );
IMPORT ER knl_mutex_initialize( void );
IMPORT ER knl_memorypool_initialize( void );
IMPORT ER knl_fix_memorypool_initialize( void );
IMPORT ER knl_cyclichandler_initialize( void );
IMPORT ER knl_alarmhandler_initialize( void );
IMPORT ER knl_subsystem_initialize( void );

/*
 * Kernel-object initialization (each object) (tkinit.c)
 */
IMPORT ER knl_init_object(void);

/*
 * Initialization of Devive management (device.c)
 */
IMPORT ER knl_initialize_devmgr( void );

/*
 * System timer control (timer.c)
 */
IMPORT ER   knl_timer_startup( void );
IMPORT void knl_timer_shutdown( void );
IMPORT void knl_timer_handler( void );

/*
 * Mutex control
 */
IMPORT void knl_signal_all_mutex( TCB *tcb );
IMPORT INT knl_chg_pri_mutex( TCB *tcb, INT priority );

/*
 * Internal memory allocation (Imalloc) (memory.c)
 */
IMPORT ER knl_init_Imalloc( void );
IMPORT void* knl_Imalloc( SZ size );
IMPORT void* knl_Icalloc( SZ nmemb, SZ size );
IMPORT void  knl_Ifree( void *ptr );

/*
 * Initial task creation parameter (inittask.c)
 */
IMPORT const T_CTSK knl_init_ctsk;

/*
 * User main program (usermain.c)
 */
IMPORT INT usermain( void );


/*
 * power-saving function
 */
IMPORT UINT	knl_lowpow_discnt;


/* ----------------------------------------------------------------------- */
/*
 * Target system-dependent routine (interrupt.c)
 */

/* Low-level memory management information (reset_hdl.c) */
IMPORT	void	*knl_lowmem_top, *knl_lowmem_limit;

/*
 * CPU control (cpu_cntl.c)
 */
#if TK_SUPPORT_REGOPS
IMPORT void knl_set_reg( TCB *tcb, CONST T_REGS *regs, CONST T_EIT *eit, CONST T_CREGS *cregs );
IMPORT void knl_get_reg( TCB *tcb, T_REGS *regs, T_EIT *eit, T_CREGS *cregs );
#endif	/* TK_SUPPORT_REGOPS */

#if TK_SUPPORT_FPU
IMPORT ER knl_get_cpr( TCB *tcb, INT copno, T_COPREGS *copregs);
IMPORT ER knl_set_cpr( TCB *tcb, INT copno, CONST T_COPREGS *copregs);
#endif

/*
 * Interuupt control (interrupt.c)
 */
IMPORT ER knl_init_interrupt( void );
#ifdef CLANGSPEC
IMPORT ER knl_define_inthdr( INT intno, ATR intatr, INTFP inthdr );
#else
IMPORT ER knl_define_inthdr( INT intno, ATR intatr, FP inthdr );
#endif /* CLANGSPEC */
IMPORT void knl_return_inthdr(void);

/*
 * Device Driver Startup / Finalization (devinit.c)
 */
IMPORT ER knl_init_device( void );
IMPORT ER knl_start_device( void );
IMPORT ER knl_finish_device( void );

/*
 * micro T-Kernel Startup / Finalization (sysinit.c)
 */
IMPORT INT main(void);
IMPORT void knl_tkernel_exit( void );

/*
 * System Call entry
 */
IMPORT void knl_call_entry( void );

/*
 *	Power-Saving Function (power.c)
 */
IMPORT void low_pow( void );		/* Switch to power-saving mode */
IMPORT void off_pow( void );		/* Move to suspend mode */

#endif /* _KERNEL_ */
