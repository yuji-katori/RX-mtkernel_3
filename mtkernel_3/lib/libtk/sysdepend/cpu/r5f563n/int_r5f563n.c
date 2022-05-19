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

#ifdef CPU_R5F563N

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
	  1,	//  21: FIFERR
	  4,	//  22: --
	  2,	//  23: FRDYI
	  4,	//  24: --
	  4,	//  25: --
	  4,	//  26: --
	  3,	//  27: SWINT
	  4,	//  28: CMI0
	  5,	//  29: CMI1
	  6,	//  30: CMI2
	  7,	//  31: CMI3
	 32,	//  32: EINT
	 33,	//  33: D0FIFO0
	 34,	//  34: D1FIFO0
	 35,	//  35: USBI0
	 36,	//  36: D0FIFO1
	 37,	//  37: D1FIFO1
	 38,	//  38: USBI1
	 39,	//  39: SPRI0
	 39,	//  40: SPTI0
	 39,	//  41: SPII0
	 42,	//  42: SPRI1
	 42,	//  43: SPTI1
	 42,	//  44: SPII1
	 45,	//  45: SPRI2
	 45,	//  46: SPTI2
	 45,	//  47: SPII2
	 48,	//  48: RXF0
	 48,	//  49: TXF0
	 48,	//  50: RXM0
	 48,	//  51: TXM0
	 52,	//  52: RXF1
	 52,	//  53: TXF1
	 52,	//  54: RXM1
	 52,	//  55: TXM1
	 56,	//  56: RXF2
	 56,	//  57: TXF2
	 56,	//  58: RXM2
	 56,	//  59: TXM2
	  4,	//  60: --
	  4,	//  61: --
	 62,	//  62: CUP
	  4,	//  63: --
	 64,	//  64: IRQ0
	 65,	//  65: IRQ1
	 66,	//  66: IRQ2
	 67,	//  67: IRQ3
	 68,	//  68: IRQ4
	 69,	//  69: IRQ5
	 70,	//  70: IRQ6
	 71,	//  71: IRQ7
	 72,	//  72: IRQ8
	 73,	//  73: IRQ9
	 74,	//  74: IRQ10
	 75,	//  75: IRQ11
	 76,	//  76: IRQ12
	 77,	//  77: IRQ13
	 78,	//  78: IRQ14
	 79,	//  79: IRQ15
	  4,	//  80: --
	  4,	//  81: --
	  4,	//  82: --
	  4,	//  83: --
	  4,	//  84: --
	  4,	//  85: --
	  4,	//  86: --
	  4,	//  87: --
	  4,	//  88: --
	  4,	//  89: --
	 90,	//  90: USBR0
	 91,	//  91: USBR1
	 92,	//  92: ALM
	 93,	//  93: PRD
	  4,	//  94: --
	  4,	//  95: --
	  4,	//  96: --
	  4,	//  97: --
	 98,	//  98: ADI0
	  4,	//  99: --
	  4,	// 100: --
	  4,	// 101: --
	102,	// 102: S12ADI0
	  4,	// 103: --
	  4,	// 104: --
	  4,	// 105: --
	106,	// 106: GROUP0
	107,	// 107: GROUP1
	108,	// 108: GROUP2
	109,	// 109: GROUP3
	110,	// 110: GROUP4
	111,	// 111: GROUP5
	112,	// 112: GROUP6
	  4,	// 113: --
	114,	// 114: GROUP12
	  4,	// 115: --
	  4,	// 116: --
	  4,	// 117: --
	  4,	// 118: --
	  4,	// 119: --
	  4,	// 120: --
	  4,	// 121: --
	122,	// 122: SCIX0
	122,	// 123: SCIX1
	122,	// 124: SCIX2
	122,	// 125: SCIX3
	126,	// 126: TGI0A
	126,	// 127: TGI0B
	126,	// 128: TGI0C
	126,	// 129: TGI0D
	130,	// 130: TGI1A
	130,	// 131: TGI1B
	132,	// 132: TGI2A
	132,	// 133: TGI2B
	134,	// 134: TGI3A
	134,	// 135: TGI3B
	134,	// 136: TGI3C
	134,	// 137: TGI3D
	138,	// 138: TGI4A
	138,	// 139: TGI4B
	140,	// 140: TGI5A
	140,	// 141: TGI5B
	142,	// 142: TGI6A(TPU6)/TGIA0(MTU0)
	142,	// 143: TGI6B(TPU6)/TGIB0(MTU0)
	142,	// 144: TGI6C(TPU6)/TGIC0(MTU0)
	142,	// 145: TGI6D(TPU6)/TGID0(MTU0)
	146,	// 146: TGIE0(MTU0)
	146,	// 147: TGIF0(MTU0)
	148,	// 148: TGI7A(TPU7)/TGIA1(MTU1)
	148,	// 149: TGI7B(TPU7)/TGIB1(MTU1)
	150,	// 150: TGI8A(TPU8)/TGIA2(MTU2)
	150,	// 151: TGI8B(TPU8)/TGIB2(MTU2)
	152,	// 152: TGI9A(TPU9)/TGIA3(MTU3)
	152,	// 153: TGI9B(TPU9)/TGIB3(MTU3)
	152,	// 154: TGI9C(TPU9)/TGIC3(MTU3)
	152,	// 155: TGI9D(TPU9)/TGID3(MTU3)
	156,	// 156: TGI10A(TPU10)/TGIA4(MTU4)
	156,	// 157: TGI10B(TPU10)/TGIB4(MTU4)
	156,	// 158: TGIC4(MTU4)
	156,	// 159: TGID4(MTU4)
	160,	// 160: TCIV4(MTU4)
	161,	// 161: TGIU5(MTU5)
	161,	// 162: TGIV5(MTU5)
	161,	// 163: TGIW5(MTU5)
	164,	// 164: TGI11A(TPU11)
	164,	// 165: TGI11B(TPU11)
	166,	// 166: OEI1
	166,	// 167: OEI2
	  4,	// 168: --
	  4,	// 169: --
	170,	// 170: CMIA0
	170,	// 171: CMIB0
	170,	// 172: OVI0
	173,	// 173: CMIA1
	173,	// 174: CMIB1
	173,	// 175: OVI1
	176,	// 176: CMIA2
	176,	// 177: CMIB2
	176,	// 178: OVI2
	179,	// 179: CMIA3
	179,	// 180: CMIB3
	179,	// 181: OVI3
	182,	// 182: EEI0
	183,	// 183: RXI0
	184,	// 184: TXI0
	185,	// 185: TEI0
	186,	// 186: EEI1
	187,	// 187: RXI1
	188,	// 188: TXI1
	189,	// 189: TEI1
	190,	// 190: EEI2
	191,	// 191: RXI2
	192,	// 192: TXI2
	193,	// 193: TEI2
	194,	// 194: EEI3
	195,	// 195: RXI3
	196,	// 196: TXI3
	197,	// 197: TEI3
	198,	// 198: DMAC0I
	199,	// 199: DMAC1I
	200,	// 200: DMAC2I
	201,	// 201: DMAC3I
	202,	// 202: EXDMAC0I
	203,	// 203: EXDMAC1I
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
	214,	// 214: RXI0
	214,	// 215: TXI0
	214,	// 216: TEI0
	217,	// 217: RXI1
	217,	// 218: TXI1
	217,	// 219: TEI1
	220,	// 220: RXI2
	220,	// 221: TXI2
	220,	// 222: TEI2
	223,	// 223: RXI3
	223,	// 224: TXI3
	223,	// 225: TEI3
	226,	// 226: RXI4
	226,	// 227: TXI4
	226,	// 228: TEI4
	229,	// 229: RXI5
	229,	// 230: TXI5
	229,	// 231: TEI5
	232,	// 232: RXI6
	232,	// 233: TXI6
	232,	// 234: TEI6
	235,	// 235: RXI7
	235,	// 236: TXI7
	235,	// 237: TEI7
	238,	// 238: RXI8
	238,	// 239: TXI8
	238,	// 240: TEI8
	241,	// 241: RXI9
	241,	// 242: TXI9
	241,	// 243: TEI9
	244,	// 244: RXI10
	244,	// 245: TXI10
	244,	// 246: TEI10
	247,	// 247: RXI11
	247,	// 248: TXI11
	247,	// 249: TEI11
	250,	// 250: RXI12
	250,	// 251: TXI12
	250,	// 252: TEI12
	253,	// 253: IEBINT
	  4,	// 254: --
	  4,	// 255: --
};

#endif /* CPU_R5F563N */