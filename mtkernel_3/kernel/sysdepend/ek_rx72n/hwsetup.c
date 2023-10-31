#include <sys/machine.h>
#ifdef EK_RX72N

#include "config.h"
#include "iodefine.h"

void HardwareSetup(void)
{
	SYSTEM.PRCR.WORD = 0xA503;				// Protect Disable

// Used Main Clock(16MHz), Uesd PLL, System Clock 240MHz
// ICLK:240MHz, PCLKA:120MHz, PCLKB:60MHz, PCLKC:60MHz, PCLKD:60MHz, FCLK:60MHz, BCLK:60MHz, UCLK:48MHz
	SYSTEM.MOSCWTCR.BYTE = 0x53;				// Main CLock Wait Time
	SYSTEM.MOSCCR.BYTE = 0x00;				// Enable Main Clock
	while( !SYSTEM.OSCOVFSR.BIT.MOOVF )  ;			// Wait Main Clock Stabilization

//	SYSTEM.PLLCR.WORD = 0x1D00;				// PLL 16MHz/1*15=240MHz
	SYSTEM.PLLCR2.BYTE = 0x00;				// Enable PLL
	while( !SYSTEM.OSCOVFSR.BIT.PLOVF )  ;			// Wait PLL Stabilization

	SYSTEM.MEMWAIT.BYTE = 0x01;				// Insert 1 Wait

	SYSTEM.SCKCR.LONG = 0x20021222;				// ICLK:240MHz,PCLKA:120MHz,FCLK=BCLK=PCLKB=PCLKC=PCLKD=60MHz
	SYSTEM.SCKCR2.WORD = 0x0041;				// UCLK:48MHz
	SYSTEM.SCKCR3.WORD = 0x0400;				// Select PLL

	SYSTEM.PPLLCR.WORD = 0x1800;				// 16MHz/1*12.5=200MHz
	SYSTEM.PPLLCR2.BYTE = 0x00;				// Enable PPLL Clock
	while( !SYSTEM.OSCOVFSR.BIT.PPLOVF )  ;			// Wait PPLL Clock Stabilization
	SYSTEM.PACKCR.WORD = 0x0011;				// CLKOUT25M is PPLL Clock
	
	SYSTEM.LOCOCR.BYTE = 0x01;				// Disable LOCO

	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
}

#pragma address __SPCCreg=0xFE7F5D40          // SPCC register
const unsigned long __SPCCreg = 0xFFFFFFFF;

#pragma address __TMEFreg=0xFE7F5D48          // TMEF register
const unsigned long __TMEFreg = 0xFFFFFFFF;

#pragma address __OSISreg=0xFE7F5D50          // OSIC register (ID codes)
const unsigned long __OSISreg[4] = {
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
        0xFFFFFFFF,
};

#pragma address __TMINFreg=0xFE7F5D10         // TMINF Register
const unsigned long __TMINFreg = 0xFFFFFFFF;

#pragma address __MDEreg=0xFE7F5D00           // MDE Register (Single Chip Mode)
#ifdef __BIG
const unsigned long __MDEreg = 0xFFFFFFF8;    // Big Endian
#else
const unsigned long __MDEreg = 0xFFFFFFFF;    // Little Endian
#endif

#pragma address __OFS0reg=0xFE7F5D04          // OFS0 Register
const unsigned long __OFS0reg = 0xFFFFFFFF;

#pragma address __OFS1reg=0xFE7F5D08          // OFS1 Register
const unsigned long __OFS1reg = 0xFFFFFFFF;

#pragma address __FAWreg=0xFE7F5D64           // FAW Register
const unsigned long __FAWreg = 0xFFFFFFFF;

#pragma address __RCPreg=0xFE7F5D70 	      // RCP register
const unsigned long __RCPreg = 0xFFFFFFFF;

#endif /* EK_RX72N */