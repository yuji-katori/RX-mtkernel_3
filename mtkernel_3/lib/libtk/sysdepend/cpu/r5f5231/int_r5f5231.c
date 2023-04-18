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

#include <tk/tkernel.h>
#include <tk/syslib.h>
#include <sys/machine.h>

#ifdef CPU_R5F5231

#include "iodefine.h"
IMPORT const UB IPRindex[];

/*
 * Enable interrupt 
 *	Enables the interrupt specified in intno.
 *	External Interrupt can be specified. 
 */
EXPORT void EnableInt( UINT intno, INT level )
{
UINT	imask;
	
	if( IPRindex[intno] != 4 )  {
		DI(imask);
		ICU.IER[intno>>3].BYTE &= ~(1<<(intno&7));
		ICU.IPR[IPRindex[intno]].BYTE = level; 
		ICU.IER[intno>>3].BYTE |= 1<<(intno&7);
		EI(imask);
	}
}

/*
 * Disable interrupt 
 *	Disables the interrupt specified in intno.
 *	External Interrupt can be specified. 
 */
EXPORT void DisableInt( UINT intno )
{
UINT	imask;
	
	if( IPRindex[intno] != 4 )  {
		DI(imask);
		ICU.IER[intno>>3].BYTE &= ~(1<<(intno&7));
		EI(imask);
	}
}

/*
 * Issue EOI to interrupt controller
 */
EXPORT void EndOfInt( UINT intno )
{
	/* No opetarion. */
}

/*
 * Clear interrupt request 
 *	Clears the intno interrupt request.
 *	Valid only for edge trigger.
 *	For edge trigger, the interrupt must be cleared with an
 *	interrupt handler.
 */
EXPORT void ClearInt( UINT intno )
{
	if( IPRindex[intno] != 4 )
		ICU.IR[intno].BIT.IR = 0;
}

/*
 * Check for interrupt requests 
 *	Checks for intvec interrupt  requests.
 *	If an interrupt request is found, returns TRUE (other than 0).
 */
EXPORT BOOL CheckInt( UINT intno )
{
	return IPRindex[intno] != 4 ? ICU.IR[intno].BIT.IR : 0;
}

/*
 * Set Interrupt Mask Level in CPU
 */
IMPORT void SetCpuIntLevel( INT level )
{
	__set_ipl( level - 1 );
}

/*
 * Get Interrupt Mask Level in CPU
 */
IMPORT INT GetCpuIntLevel( void )
{
	return __get_ipl( ) + 1;
}

const UB IPRindex[]={
	  4,	//   0: --
	  4,	//   1: --
	  4,	//   2: --
	  4,	//   3: --
	  4,	//   4: --
	  4,	//   5: --
	  4,	//   6: --
	  4,	//   7: --
	  4,	//   8: --
	  4,	//   9: --
	  4,	//  10: --
	  4,	//  11: --
	  4,	//  12: --
	  4,	//  13: --
	  4,	//  14: --
	  4,	//  15: --
	  0,	//  16: BUSERR
	  4,	//  17: --
	  4,	//  18: --
	  4,	//  19: --
	  4,	//  20: --
	  4,	//  21: --
	  4,	//  22: --
	  2,	//  23: FCU      FRDYI
	  4,	//  24: --
	  4,	//  25: --
	  4,	//  26: --
	  3,	//  27: ICU      SWINT
	  4,	//  28: CMT0     CMI0
	  5,	//  29: CMT1     CMI1
	  6,	//  30: CMT2     CMI2
	  7,	//  31: CMT3     CMI3
	 32,	//  32: CAC      FERRF
	 33,	//  33:          MENDF
	 34,	//  34:          OVFF
	  4,	//  35: --
	 36,	//  36: USB      D0FIFO0
	 37,	//  37:          D1FIFO0
	 38,	//  38:          USBI0
	  4,	//  39: --
	 40,	//  40: SDHI     SBFAI
	 41,	//  41:          CDETI
	 42,	//  42:          CACI
	 43,	//  43:          SDACI
	 44,	//  44: RSPI0    SPEI0
	 44,	//  45:          SPRI0
	 44,	//  46:          SPTI0
	 44,	//  47:          SPII0
	  4,	//  48: --
	  4,	//  49: --
	  4,	//  50: --
	  4,	//  51: --
	 52,	//  52: CAN      COMFRXINT
	 53,	//  53:          RXFINT
	 54,	//  54:          TXINT
	 55,	//  55:          CHERRINT
	 56,	//  56:          GLERRINT
	 57,	//  57: DOC      DOPCF
	 58,	//  58: CMPB     CMPB0
	 59,	//  59:          CMPB1
	 60,	//  60: CTSU     CTSUWR
	 60,	//  61:          CTSURD
	 60,	//  62:          CTSUFN
	 63,	//  63: RTC      CUP
	 64,	//  64: ICU      IRQ0
	 65,	//  65:          IRQ1
	 66,	//  66:          IRQ2
	 67,	//  67:          IRQ3
	 68,	//  68:          IRQ4
	 69,	//  69:          IRQ5
	 70,	//  70:          IRQ6
	 71,	//  71:          IRQ7
	  4,	//  72: --
	  4,	//  73: --
	  4,	//  74: --
	  4,	//  75: --
	  4,	//  76: --
	  4,	//  77: --
	  4,	//  78: --
	  4,	//  79: --
	 80,	//  80: ELC      ELSR8I
	  4,	//  81: --
	  4,	//  82: --
	  4,	//  83: --
	  4,	//  84: --
	  4,	//  85: --
	  4,	//  86: --
	  4,	//  87: --
	 88,	//  88: LVD/CMPA LVD1/CMPA1
	 89,	//  89:          LVD2/CMPA2
	 90,	//  90: USB0     USBR0
	 91,	//  91: VBATT    VBTLVDI
	 92,	//  92: RTC      ALM
	 93,	//  93:          PRD
	  4,	//  94: --
	  4,	//  95: --
	  4,	//  96: --
	  4,	//  97: --
	  4,	//  98: --
	  4,	//  99: --
	  4,	// 100: --
	  4,	// 101: --
	102,	// 102: S12AD    S12ADI0
	103,	// 103:          GBADI
	104,	// 104: CMPB1    CMPB2
	105,	// 105:          CMPB3
	106,	// 106: ELC      ELSR18I
	107,	// 107:          ELSR19I
	108,	// 108: SSI0     SSIF0
	108,	// 109:          SSIRXI0
	108,	// 110:          SSITXI0
	111,	// 111: Security RD
	111,	// 112:          WR
	113,	// 113:          Error
	114,	// 114: MTU0     TGIA0
	114,	// 115:          TGIB0
	114,	// 116:          TGIC0
	114,	// 117:          TGID0
	118,	// 118:          TCIV0
	118,	// 119:          TGIE0
	118,	// 120:          TGIF0
	121,	// 121: MTU1     TGIA1
	121,	// 122:          TGIB1
	123,	// 123:          TCIV1
	123,	// 124:          TCIU1
	125,	// 125: MTU2     TGIA2
	125,	// 126:          TGIB2
	127,	// 127:          TCIV2
	127,	// 128:          TCIU2
	129,	// 129: MTU3     TGIA3
	129,	// 130:          TGIB3
	129,	// 131:          TGIC3
	129,	// 132:          TGID3
	133,	// 133:          TCIV3
	134,	// 134: MTU4     TGIA4
	134,	// 135:          TGIB4
	134,	// 136:          TGIC4
	134,	// 137:          TGID4
	138,	// 138:          TCIV4
	139,	// 139: MTU5     TGIU5
	139,	// 140:          TGIV5
	139,	// 141:          TGIW5
	142,	// 142: TPU0     TGI0A
	142,	// 143:          TGI0B
	142,	// 144:          TGI0C
	142,	// 145:          TGI0D
	146,	// 146:          TCI0V
	147,	// 147: TPU1     TGI1A
	147,	// 148:          TGI1B
	149,	// 149:          TCI1V
	149,	// 150:          TCI1U
	151,	// 151: TPU2     TGI2A
	151,	// 152:          TGI2B
	153,	// 153:          TCI2V
	153,	// 154:          TCI2U
	155,	// 155: TPU3     TGI3A
	155,	// 156:          TGI3B
	155,	// 157:          TGI3C
	155,	// 158:          TGI3D
	159,	// 159:          TCI3V
	160,	// 160: TPU4     TGI4A
	160,	// 161:          TGI4B
	162,	// 162:          TCI4V
	162,	// 163:          TCI4U
	164,	// 164: TPU5     TGI5A
	164,	// 165:          TGI5B
	166,	// 166:          TCI5V
	166,	// 167:          TCI5U
	  4,	// 168: --
	  4,	// 169: --
	170,	// 170: POE      OEI1
	171,	// 171:          OEI2
	  4,	// 172: --
	  4,	// 173: --
	174,	// 174: TMR0     CMIA0
	174,	// 175:          CMIB0
	174,	// 176:          OVI0
	177,	// 177: TMR1     CMIA1
	177,	// 178:          CMIB1
	177,	// 179:          OVI1
	180,	// 180: TMR2     CMIA2
	180,	// 181:          CMIB2
	180,	// 182:          OVI2
	183,	// 183: TMR3     CMIA3
	183,	// 184:          CMIB3
	183,	// 185:          OVI3
	  4,	// 186: --
	  4,	// 187: --
	  4,	// 188: --
	  4,	// 189: --
	  4,	// 190: --
	  4,	// 191: --
	  4,	// 192: --
	  4,	// 193: --
	  4,	// 194: --
	  4,	// 195: --
	  4,	// 196: --
	  4,	// 197: --
	198,	// 198: DMAC     DMAC0I
	199,	// 199:          DMAC1I
	200,	// 200:          DMAC2I
	201,	// 201:          DMAC3I
	  4,	// 202: --
	  4,	// 203: --
	  4,	// 204: --
	  4,	// 205: --
	  4,	// 206: --
	  4,	// 207: --
	  4,	// 208: --
	  4,	// 209: --
	  4,	// 210: --
	  4,	// 211: --
	  4,	// 212: --
	  4,	// 213: --
	214,	// 214: SCI0     ERI0
	214,	// 215:          RXI0
	214,	// 216:          TXI0
	214,	// 217:          TEI0
	218,	// 218: SCI1     ERI1
	218,	// 219:          RXI1
	218,	// 220:          TXI1
	218,	// 221:          TEI1
	222,	// 222: SCI5     ERI5
	222,	// 223:          RXI5
	222,	// 224:          TXI5
	222,	// 225:          TEI5
	226,	// 226: SCI6     ERI6
	226,	// 227:          RXI6
	226,	// 228:          TXI6
	226,	// 229:          TEI6
	230,	// 230: SCI8     ERI8
	230,	// 231:          RXI8
	230,	// 232:          TXI8
	230,	// 233:          TEI8
	234,	// 234: SCI9     ERI9
	234,	// 235:          RXI9
	234,	// 236:          TXI9
	234,	// 237:          TEI9
	238,	// 238: SCI12    ERI12
	238,	// 239:          RXI12
	238,	// 240:          TXI12
	238,	// 241:          TEI12
	242,	// 242:          SCIX0
	243,	// 243:          SCIX1
	244,	// 244:          SCIX2
	245,	// 245:          SCIX3
	246,	// 246: RIIC0    EEI0
	247,	// 247:          RXI0
	248,	// 248:          TXI0
	249,	// 249:          TEI0
	  4,	// 250: --
	  4,	// 251: --
	  4,	// 252: --
	  4,	// 253: --
	  4,	// 254: --
	  4,	// 255: --
};

#endif /* CPU_R5F5231 */