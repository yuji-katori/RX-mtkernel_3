#include <sys/machine.h>
#ifdef AP_RX63N

#include "config.h"
#include "iodefine.h"

void HardwareSetup(void)
{
int i;
	// If Used 12MHz Main Clock
	SYSTEM.PRCR.WORD = 0xA503;				// Protect Disable

	SYSTEM.MOSCWTCR.BYTE = 0x0D;				// 12MHz*13107cyc=10.9225ms
	SYSTEM.MOSCCR.BYTE = 0x00;				// Enable Main Clock

	SYSTEM.PLLCR.WORD = 0x0F00;				// PLL 12MHz*16/1=192MHz
	SYSTEM.PLLCR2.BYTE = 0x00;				// Enable PLL
	SYSTEM.PLLWTCR.BYTE = 0x0E;				// 192MHz*2097152cyc=10.9226ms
	for( i=0 ; i<719  ; i++ )  ;				// 20ms/143.75kHz/4cyc=718.75

	SYSTEM.SCKCR.LONG = 0x61821211;				// ICLK=96MHz,PCLK=48MHz,BCLK=48MHz
	SYSTEM.SCKCR2.WORD = 0x0032;				// UCLK=48MHz,IECLK=48MHz
	SYSTEM.SCKCR3.WORD = 0x0400;				// Select PLL

	SYSTEM.LOCOCR.BYTE = 0x01;				// Disable LOCO

	BSC.CS1MOD.WORD = 0x0001;				// 1 Write Strobe
	BSC.CS1WCR1.LONG = 0x02020000;
	BSC.CS1WCR2.LONG = 0x00110111;
	BSC.CS1CR.WORD = 0x0001;				// CS1 Enable
	BSC.CSRECEN.WORD = 0x3E3E;
	
	MPC.PFCSE.BYTE = 0x02;					// CS1 Output Enable
	MPC.PFAOE0.BYTE = 0xFF;					// A8-A15 Enable
	MPC.PFAOE1.BYTE = 0x0F;					// A16-A19 Enable A20-A23 Disable

	MPC.PFBCR0.BYTE = 0x51;					// P51 => WR1 PE0-PE7 => D8-D15 PA0-PA7 => A0-A7

	SYSTEM.SYSCR0.WORD = 0x5A03;				// Enable External Bus
	while( SYSTEM.SYSCR0.BIT.EXBE == 0 )  ;

	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
}
#endif /* AP_RX63N */