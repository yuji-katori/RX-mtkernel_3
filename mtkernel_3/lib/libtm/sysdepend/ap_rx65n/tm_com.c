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
 *    tm_com.c
 *    T-Monitor Communication low-level device driver (AP-RX65N-0A)
 */

#include <tk/typedef.h>
#include <sys/sysdef.h>
#include "../../libtm.h"

#include "iodefine.h"
#define BAUD_RATE	(115200)			/* 115.2kbps */

#ifdef CLANGSPEC
EXPORT	void	tm_snd_dat( const VB* buf, INT size )
#else
EXPORT	void	tm_snd_dat( const UB* buf, INT size )
#endif /* CLANGSPEC */
{
#if USE_COM
int i;
	for( i=0 ; i < size ; i++ )  {
		while(!IR(SCI6, TXI6))
			;
		IR(SCI6, TXI6) = 0;
		SCI6.TDR = buf[i];
	}
#endif /* USE_COM */
}


#ifdef CLANGSPEC
EXPORT	void	tm_rcv_dat( VB* buf, INT size )
#else
EXPORT	void	tm_rcv_dat( UB* buf, INT size )
#endif /* CLANGSPEC */
{
#if USE_COM
int i;
	for( i=0 ; i < size ; i++ )
		while( 1 )  {
			if(IR(SCI6, RXI6))  {
				buf[i] = SCI6.RDR;
				IR(SCI6, RXI6) = 0;
				break;
			}
			if(SCI6.SSR.BYTE & 0x38)
				SCI6.SSR.BYTE = 0xC0;
		}
#endif /* USE_COM */
}


EXPORT	void	tm_com_init(void)
{
#if USE_COM
int i;
	SYSTEM.PRCR.WORD = 0xA502;
	MSTP( SCI6 ) = 0;
	SYSTEM.PRCR.WORD = 0xA500;
	
	MPC.PWPR.BIT.B0WI = 0;
	MPC.PWPR.BIT.PFSWE = 1;
	MPC.P00PFS.BIT.PSEL = 0x0A;
	MPC.P01PFS.BIT.PSEL = 0x0A;
	MPC.PWPR.BYTE = 0x80;

	// GPIO setting P00 = TX6, P01 = RX6
	PORT0.PMR.BIT.B0 = 1;
	PORT0.PMR.BIT.B1 = 1;

	// Initialize SCI6
	SCI6.BRR = PCLKB / (64.0F / 2 * BAUD_RATE) - 0.5F;
	for( i=0 ; i<ICLK/4/BAUD_RATE  ; i++ )  ;
	SCI6.SCR.BYTE |= 0xF0;
#endif /* USE_COM */
}