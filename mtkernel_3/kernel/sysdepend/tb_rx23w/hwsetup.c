#include <sys/machine.h>
#ifdef TB_RX23W

#include "config.h"
#include "iodefine.h"

void HardwareSetup(void)
{
	SYSTEM.PRCR.WORD = 0xA503;				// Protect Disable

// Used Main Clock(54MHz), System Clock 54MHz
// ICLK:54MHz, PCLKA:54MHz, PCLKB:27MHz, PCLKD:54MHz, FCLK=27MHz
	SYSTEM.HOCOCR2.BYTE = 0x03;				// Select 54MHz
	if( SYSTEM.HOCOCR.BIT.HCSTP )  {			// HOCO is Stabilization ?
		SYSTEM.HOCOCR.BIT.HCSTP = 0;			// HOCO is Stabilization
		while( ! SYSTEM.OSCOVFSR.BIT.HCOVF )  ;		// Wait for HOCO is Stabilization
	}

	SYSTEM.MEMWAIT.BIT.MEMWAIT = 1;				// Set ROM wait time to 1 for 54MHz 
	while( ! SYSTEM.MEMWAIT.BIT.MEMWAIT )  ;		// Wait for Change Bit

	SYSTEM.SCKCR.LONG = 0x10030100;				// ICLK=PCLKA=PCLKD:54MHz,PCLKB=FCLK:27MHz
	SYSTEM.SCKCR3.WORD = 0x0100;				// Select HOCO

	SYSTEM.LOCOCR.BYTE = 0x01;				// Disable LOCO
	RTC.RCR3.BIT.RTCEN = 0;					// Disable Sub Clock
	SYSTEM.SOSCCR.BYTE = 0x01;				// Disable Sub Clock

	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
}

#endif /* TB_RX23W */