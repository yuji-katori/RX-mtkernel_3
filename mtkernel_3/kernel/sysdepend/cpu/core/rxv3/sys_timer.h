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
 *	sys_timer.h (RXv3)
 *	Hardware-Dependent System Timer (SysTick) Processing
 */

#ifndef _SYSDEPEND_CPU_CORE_SYSTIMER_
#define _SYSDEPEND_CPU_CORE_SYSTIMER_

#include "iodefine.h"
#define CMCR_CKS	2		/* 0:PCLK/8, 1:PCLK/32, 2:PCLK/128, 3:PCLK/512 */

/*
 * Set timer
 */
Inline void knl_init_hw_timer( void )
{
	UINT	imask;
	DI(imask);

	SYSTEM.PRCR.WORD = 0xA502;			/* Protect Disable */
	MSTP( CMT0 ) = 0;				/* Disable Module Stop of CMT0, CMT1 */
	SYSTEM.PRCR.WORD = 0xA500;			/* Protect Enable */
	CMT0.CMCOR = PCLKB/1000*CFN_TIMER_PERIOD/(1<<((CMCR_CKS+1)*2+1)) - 1;	/* Set Timer Count */
	CMT0.CMCR.WORD = 0x0040 | CMCR_CKS;		/* Interrupt is Enable, Set Frequency Dividing */
	IPR( CMT0, CMI0 ) = TIMER_INTLEVEL;		/* Set Interrupt Level */
	IEN( CMT0, CMI0 ) = 1;				/* Interrupt Enable */
	CMT.CMSTR0.BIT.STR0 = 1;			/* Start Timer Count */

	EI(imask);
}

/*
 * Timer start processing
 *	Initialize the timer and start the periodical timer interrupt.
 */
Inline void knl_start_hw_timer( void )
{
	/* Set timer */
	knl_init_hw_timer();
}

/*
 * Clear timer interrupt
 *	Clear the timer interrupt request. Depending on the type of
 *	hardware, there are two timings for clearing: at the beginning
 *	and the end of the interrupt handler.
 *	'clear_hw_timer_interrupt()' is called at the beginning of the
 *	timer interrupt handler.
 *	'end_of_hw_timer_interrupt()' is called at the end of the timer
 *	interrupt handler.
 *	Use either or both according to hardware.
 */
Inline void knl_clear_hw_timer_interrupt( void )
{
	/* Nothing required to do at this point */
}

Inline void knl_end_of_hw_timer_interrupt( void )
{
	/* Nothing required to do at this point */
}

/*
 * Timer stop processing
 *	Stop the timer operation.
 *	Called when system stops.
 */
Inline void knl_terminate_hw_timer( void )
{
	IEN( CMT0, CMI0 ) = 0;			/* Disable Timer Interrupt */
	CMT0.CMCR.WORD = 0;			/* Disable Compare Match Interrupt */
	CMT.CMSTR0.BIT.STR0 = 0;		/* Stop Timer Count */
	SYSTEM.PRCR.WORD = 0xA502;		/* Protect Disable */
	MSTP( CMT0 ) = 1;			/* Enable Module Stop of CMT0, CMT1 */
	SYSTEM.PRCR.WORD = 0xA500;		/* Protect Enable */
}

/*
 * Get processing time from the previous timer interrupt to the
 * current (nanosecond)
 *	Consider the possibility that the timer interrupt occurred
 *	during the interrupt disable and calculate the processing time
 *	within the following
 *	range: 0 <= Processing time < TIMER_PERIOD * 2
 */
Inline UW knl_get_hw_timer_nsec( void )
{
	/* Nothing required to do at this point */
	return 0;
}

#endif /* _SYSDEPEND_CPU_CORE_SYSTIMER_ */
