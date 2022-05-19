#include <sys/machine.h>
#ifdef GR_SAKURA

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

	SYSTEM.PRCR.WORD = 0xA500;				// Protect Enable
}
#endif /* GR_SAKURA */