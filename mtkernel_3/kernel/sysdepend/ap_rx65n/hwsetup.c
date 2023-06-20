/*
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2022/11/2.
 *    Modified by Yuji Katori at 2023/5/16.
 *----------------------------------------------------------------------
 */
#include <sys/machine.h>
#ifdef AP_RX65N

#include "config.h"
#include "iodefine.h"

void HardwareSetup(void)
{
	SYSTEM.PRCR.WORD = 0xA503;				// Protect Disable

// Used Main Clock(24MHz), Uesd PLL, System Clock 240MHz
// ICLK:120MHz, PCLKA:120MHz, PCLKB:60MHz, PCLKC:60MHz, PCLKD:60MHz, FCLK:60MHz, BCLK:60MHz, UCLK:48MHz
	SYSTEM.MOFCR.BYTE = 0x00;				// 24MHz XTAL
	SYSTEM.MOSCWTCR.BYTE = 0x5C;				// Main CLock Wait Time(from RSK Sample)
	SYSTEM.MOSCCR.BYTE = 0x00;				// Enable Main Clock
	while( !SYSTEM.OSCOVFSR.BIT.MOOVF )  ;			// Wait Main Clock Stabilization
	
	SYSTEM.ROMWT.BIT.ROMWT = 0x2;				// Set ROM wait time to 2 for 120MHz 

	SYSTEM.PLLCR.WORD = 0x1300;				// PLL 24MHz/1*10=240MHz
	SYSTEM.PLLCR2.BYTE = 0x00;				// Enable PLL
	while( !SYSTEM.OSCOVFSR.BIT.PLOVF )  ;			// Wait PLL Stabilization

	SYSTEM.SCKCR.LONG = 0x21021222;				// ICLK=PCLKA:120MHz,FCLK=BCLK=PCLKB=PCLKC=PCLKD=60MHz
	SYSTEM.SCKCR2.WORD = 0x0041;				// UCLK:48MHz
	SYSTEM.SCKCR3.WORD = 0x0400;				// Select PLL

	SYSTEM.LOCOCR.BYTE = 0x01;				// Disable LOCO
	RTC.RCR3.BIT.RTCEN = 0;					// Disable Sub Clock
	SYSTEM.SOSCCR.BYTE = 0x01;				// Disable Sub Clock
	while ( SYSTEM.OSCOVFSR.BIT.SOOVF )  ;			// Wait Sub Clock Stoped

	BSC.SDIR.WORD = 0x0020;					// Set Auto Refresh
	BSC.SDICR.BIT.INIRQ = 1; 				// Initialize Request
	while( BSC.SDSR.BYTE != 0x00 )  ;			// Wait Initialize End
//	BSC.SDCCR.BYTE = 0x00;
	BSC.SDMOD.WORD = 0x0230;				// Set Mode
	BSC.SDTR.LONG = 0x00021203;				// Set Timing
	BSC.SDADR.BYTE = 0x01;					// Set 9 bit Shift
//	BSC.SDAMOD.BYTE = 0x00;
//	BSC.SDCMOD.BYTE = 0x00;
	BSC.SDRFCR.WORD = 0x43A8;				// Set Refresh Count
	BSC.SDRFEN.BIT.RFEN = 1;				// Set Auto Refresh
	BSC.SDCCR.BIT.EXENB = 1;				// SDRAM Enable

	MPC.PFAOE0.BYTE = 0xFF;					// Enable A8-A15
	MPC.PFBCR0.BYTE |= 0x11;				// Enable A0-A7,D8-D15
	MPC.PFBCR1.BYTE |= 0xD0;				// Enable SDRAM Pin
	SYSTEM.SYSCR0.WORD = 0x5A03;				// Enable External Bus

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

#pragma address __ROMCODEreg=0xFE7F5D70       // ROMCODE Register
const unsigned long __ROMCODEreg = 0xFFFFFFFF;

#endif /* AP_RX65N */