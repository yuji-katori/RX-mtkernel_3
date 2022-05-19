/*
 *----------------------------------------------------------------------
 *    UCT micro T-Kernel DevKit tuned for C_First
 *
 *    Copyright (c) 2012 UC Technology. All Rights Reserved.
 *----------------------------------------------------------------------
 *
 *    Version:   1.01.02
 *    Released by UC Technology (http://www.uctec.com/) at 2021/04/08.
 *
 *----------------------------------------------------------------------
 */
#include <sys/machine.h>
#ifdef CPU_CORE_RXV1

#include <tk/tkernel.h>
#include <config.h>

ID	const	cfn_max_tskid = MAX_TSKID;
ID	const	cfn_max_semid = MAX_SEMID;
ID	const	cfn_max_flgid = MAX_FLGID;
ID	const	cfn_max_mbxid = MAX_MBXID;
ID	const	cfn_max_mtxid = MAX_MTXID;
ID	const	cfn_max_mbfid = MAX_MBFID;
ID	const	cfn_max_mplid = MAX_MPLID;
ID	const	cfn_max_mpfid = MAX_MPFID;
ID	const	cfn_max_cycid = MAX_CYCID;
ID	const	cfn_max_almid = MAX_ALMID;

#endif	/* CPU_CORE_RXV1 */