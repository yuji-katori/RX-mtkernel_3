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
 *    T-Monitor Communication low-level device driver (TB-RX66N)
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
		while(!IR(SCI9, TXI9))
			;
		IR(SCI9, TXI9) = 0;
		SCI9.TDR = buf[i];
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
			if(IR(SCI9, RXI9))  {
				buf[i] = SCI9.RDR;
				IR(SCI9, RXI9) = 0;
				break;
			}
			if(SCI9.SSR.BYTE & 0x38)
				SCI9.SSR.BYTE = 0xC0;
		}
#endif /* USE_COM */
}


EXPORT	void	tm_com_init(void)
{
#if USE_COM
int i;
	SYSTEM.PRCR.WORD = 0xA502;
	MSTP( SCI9 ) = 0;
	SYSTEM.PRCR.WORD = 0xA500;
	
	MPC.PWPR.BIT.B0WI = 0;
	MPC.PWPR.BIT.PFSWE = 1;
	MPC.PB7PFS.BIT.PSEL = 0x0A;
	MPC.PB6PFS.BIT.PSEL = 0x0A;
	MPC.PWPR.BYTE = 0x80;

	// GPIO setting PB7 = TXD9, PB6 = RXD9
	PORTB.PMR.BIT.B7 = 1;
	PORTB.PMR.BIT.B6 = 1;

	// Initialize SCI9
	SCI9.SEMR.BIT.ABCSE = 1;
	SCI9.BRR = PCLKA / (12.0F / 2 * BAUD_RATE) - 0.5F;
	for( i=0 ; i<ICLK/4/BAUD_RATE  ; i++ )  ;
	SCI9.SCR.BYTE |= 0xF0;
#endif /* USE_COM */
}