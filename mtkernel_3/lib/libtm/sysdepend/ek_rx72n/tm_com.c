/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2006-2023 by Ken Sakamura.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 *
 *    Released by TRON Forum(http://www.tron.org) at 2019/12/11.
 *
 *----------------------------------------------------------------------
 */

/*
 *    tm_com.c
 *    T-Monitor Communication low-level device driver (EK-RX72N)
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
		while(!IR(SCI2, TXI2))
			;
		IR(SCI2, TXI2) = 0;
		SCI2.TDR = buf[i];
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
			if(IR(SCI2, RXI2))  {
				buf[i] = SCI2.RDR;
				IR(SCI2, RXI2) = 0;
				break;
			}
			if(SCI2.SSR.BYTE & 0x38)
				SCI2.SSR.BYTE = 0xC0;
		}
#endif /* USE_COM */
}


EXPORT	void	tm_com_init(void)
{
#if USE_COM
int i;	
	SYSTEM.PRCR.WORD = 0xA502;
	MSTP( SCI2 ) = 0;
	SYSTEM.PRCR.WORD = 0xA500;
	
	MPC.PWPR.BIT.B0WI = 0;
	MPC.PWPR.BIT.PFSWE = 1;
	MPC.P13PFS.BIT.PSEL = 0x0A;
	MPC.P12PFS.BIT.PSEL = 0x0A;
	MPC.PWPR.BYTE = 0x80;

	// GPIO setting P13 = TXD2, P12 = RXD2
	PORT1.PMR.BIT.B3 = 1;
	PORT1.PMR.BIT.B2 = 1;

	// Initialize SCI2
	SCI2.SEMR.BIT.ABCSE = 1;
	SCI2.BRR = PCLKB / (12.0F / 2 * BAUD_RATE) - 0.5F;
	for( i=0 ; i<ICLK/4/BAUD_RATE  ; i++ )  ;
	SCI2.SCR.BYTE |= 0xF0;
#endif /* USE_COM */
}