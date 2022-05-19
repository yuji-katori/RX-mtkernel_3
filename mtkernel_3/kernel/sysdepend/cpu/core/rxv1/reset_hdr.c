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
#include <sys/machine.h>
#ifdef CPU_CORE_RXV1

/*
 *	reset_hdr.c (RXv1)
 *	Reset handler
 */

#include "kernel.h"
#include "../../../sysdepend.h"

#include <tm/tmonitor.h>


#if USE_IMALLOC
/* Low level memory manager information */
EXPORT	void	*knl_lowmem_top;		// Head of area (Low address)
EXPORT	void	*knl_lowmem_limit;		// End of area (High address)
#endif

EXPORT void Reset_Handler(void)
{
	/* Startup Hardware */
	knl_startup_device();

#if USE_IMALLOC
	/* Set System memory area */
#if CFN_SYSTEMAREA_TOP == 0
	knl_lowmem_top = (void *)__secend("R");
#else
	knl_lowmem_top = (void *)CFN_SYSTEMAREA_TOP;
#endif
#if CFN_SYSTEMAREA_END == 0
	knl_lowmem_limit = (void *)(INTERNAL_RAM_END - EXC_STACK_SIZE);
#else
	knl_lowmem_limit = (void *)(CFN_SYSTEMAREA_END - EXC_STACK_SIZE);
#endif
#endif

	/* Startup Kernel */
	main();		/**** No return ****/
	while(1);	/* guard - infinite loops */
}

#endif	/* CPU_CORE_RXV1 */