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

#ifdef CPU_R5F565N

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
	  0,	//  16: BSC    BUSERR
	  4,	//  17: --
	  0,	//  18: RAM    RAMERR
	  4,	//  19: --
	  4,	//  20: --
	  1,	//  21: FCU    FIFERR
	  4,	//  22: --
	  2,	//  23: FCU    FRDYI
	  4,	//  24: --
	  4,	//  25: --
	  3,	//  26: ICU    SWINT2
	  3,	//  27:        SWINT
	  4,	//  28: CMT0   CMI0
	  5,	//  29: CMT1   CMI1
	  6,	//  30: CMTW0  CMWI0
	  7,	//  31: CMTW1  CMWI1
	  4,	//  32: --
	  4,	//  33: --
	 34,	//  34: USB0   D0FIFO0
	 35,	//  35:        D1FIFO0
	  4,	//  36: --
	  4,	//  37: --
	 38,	//  38: RSPI0  SPRI0
	 39,	//  39:        SPTI0
	 40,	//  40: RSPI1  SPRI1
	 41,	//  41:        SPRI1
	 42,	//  42: QSPI   SPRI
	 43,	//  43:        SPTI
	 44,	//  44: SDHI   SBFAI
	 45,	//  45: MMCIF  MBFAI
	  4,	//  46: --
	  4,	//  47: --
	  4,	//  48: --
	  4,	//  49: --
	 50,	//  50: SRC    IDEI
	 51,	//  51:        ODFI
	 52,	//  52: RIIC0  RXI0
	 53,	//  53:        TXI0
	 54,	//  54: RIIC2  RXI2
	 55,	//  55:        TXI2
	  4,	//  56: --
	  4,	//  57: --
	 58,	//  58: SCI0   RXI0
	 59,	//  59:        TXI0
	 60,	//  60: SCI1   RXI1
	 61,	//  61:        TXI1
	 62,	//  62: SCI2   RXI2
	 63,	//  63:        TXI2
	 64,	//  64: ICU    IRQ0
	 65,	//  65:        IRQ1
	 66,	//  66:        IRQ2
	 67,	//  67:        IRQ3
	 68,	//  68:        IRQ4
	 69,	//  69:        IRQ5
	 70,	//  70:        IRQ6
	 71,	//  71:        IRQ7
	 72,	//  72:        IRQ8
	 73,	//  73:        IRQ9
	 74,	//  74:        IRQ10
	 75,	//  75:        IRQ11
	 76,	//  76:        IRQ12
	 77,	//  77:        IRQ13
	 78,	//  78:        IRQ14
	 79,	//  79:        IRQ15
	 80,	//  80: SCI3   RXI3
	 81,	//  81:        TXI3
	 82,	//  82: SCI4   RXI4
	 83,	//  83:        TXI4
	 84,	//  84: SCI5   RXI5
	 85,	//  85:        TXI5
	 86,	//  86: SCI6   RXI6
	 87,	//  87:        TXI7
	 88,	//  88: LVD1   LVD1
	 89,	//  89: LVD2   LVD2
	 90,	//  90: USB0   USBR0
	  4,	//  91: --
	 92,	//  92: RTC    ALM
	 93,	//  93:        PRD
	  4,	//  94: --
	 95,	//  95: IWDT   IWUNI
	 96,	//  96: WDT    WUNI
	 97,	//  97: PDC    PCDFI
	 98,	//  98: SCI7   RXI7
	 99,	//  99:        TXI7
	100,	// 100: SCI8   RXI8
	101,	// 101:        TXI8
	102,	// 102: SCI9   RXI9
	103,	// 103:        TXI9
	104,	// 104: SCI10  RXI10
	105,	// 105:        TXI10
	106,	// 106: ICU    GROUPBE0
	107,	// 107: ICU    GROUPBL2
	108,	// 108: RSPI2  SPRI2
	109,	// 109:        SPTI2
	110,	// 110:        GROUPBL0
	111,	// 111:        GROUPBL1
	112,	// 112:        GROUPAL0
	113,	// 113:        GROUPAL1
	114,	// 114: SCI11  RXI11
	115,	// 115:        TXI11
	116,	// 116: SCI12  RXI12
	117,	// 117:        TXI12
	  4,	// 118: --
	  4,	// 119: --
	120,	// 120: DMAC   DMAC0I
	121,	// 121:        DMAC1I
	122,	// 122:        DMAC2I
	123,	// 123:        DMAC3I
	124,	// 124:        DMAC74I
	125,	// 125: OST    OSTDI
	126,	// 126: EXDMAC EXDMAC0I
	127,	// 127:        EXDMAC1I
	128,	// 128: PERIB  INTB128
	129,	// 129:        INTB129
	130,	// 130:        INTB130
	131,	// 131:        INTB131
	132,	// 132:        INTB132
	133,	// 133:        INTB133
	134,	// 134:        INTB134
	135,	// 135:        INTB135
	136,	// 136:        INTB136
	137,	// 137:        INTB137
	138,	// 138:        INTB138
	139,	// 139:        INTB139
	140,	// 140:        INTB140
	141,	// 141:        INTB141
	142,	// 142:        INTB142
	143,	// 143:        INTB143
	144,	// 144:        INTB144
	145,	// 145:        INTB145
	146,	// 146:        INTB146
	147,	// 147:        INTB147
	148,	// 148:        INTB148
	149,	// 149:        INTB149
	150,	// 150:        INTB150
	151,	// 151:        INTB151
	152,	// 152:        INTB152
	153,	// 153:        INTB153
	154,	// 154:        INTB154
	155,	// 155:        INTB155
	156,	// 156:        INTB156
	157,	// 157:        INTB157
	158,	// 158:        INTB158
	159,	// 159:        INTB159
	160,	// 160:        INTB160
	161,	// 161:        INTB161
	162,	// 162:        INTB162
	163,	// 163:        INTB163
	164,	// 164:        INTB164
	165,	// 165:        INTB165
	166,	// 166:        INTB166
	167,	// 167:        INTB167
	168,	// 168:        INTB168
	169,	// 169:        INTB169
	170,	// 170:        INTB170
	171,	// 171:        INTB171
	172,	// 172:        INTB172
	173,	// 173:        INTB173
	174,	// 174:        INTB174
	175,	// 175:        INTB175
	176,	// 176:        INTB176
	177,	// 177:        INTB177
	178,	// 178:        INTB178
	179,	// 179:        INTB179
	180,	// 180:        INTB180
	181,	// 181:        INTB181
	182,	// 182:        INTB182
	183,	// 183:        INTB183
	184,	// 184:        INTB184
	185,	// 185:        INTB185
	186,	// 186:        INTB186
	187,	// 187:        INTB187
	188,	// 188:        INTB188
	189,	// 189:        INTB189
	190,	// 190:        INTB190
	191,	// 191:        INTB191
	192,	// 192:        INTB192
	193,	// 193:        INTB193
	194,	// 194:        INTB194
	195,	// 195:        INTB195
	196,	// 196:        INTB196
	197,	// 197:        INTB197
	198,	// 198:        INTB198
	199,	// 199:        INTB199
	200,	// 200:        INTB200
	201,	// 201:        INTB201
	202,	// 202:        INTB202
	203,	// 203:        INTB203
	204,	// 204:        INTB204
	205,	// 205:        INTB205
	206,	// 206:        INTB206
	207,	// 207:        INTB207
	208,	// 208: PERIA  INTA208
	209,	// 209:        INTA209
	210,	// 210:        INTA210
	211,	// 211:        INTA211
	212,	// 212:        INTA212
	213,	// 213:        INTA213
	214,	// 214:        INTA214
	215,	// 215:        INTA215
	216,	// 216:        INTA216
	217,	// 217:        INTA217
	218,	// 218:        INTA218
	218,	// 219:        INTA219
	220,	// 220:        INTA220
	221,	// 221:        INTA221
	222,	// 222:        INTA222
	223,	// 223:        INTA223
	224,	// 224:        INTA224
	225,	// 225:        INTA225
	226,	// 226:        INTA226
	227,	// 227:        INTA227
	228,	// 228:        INTA228
	229,	// 229:        INTA229
	230,	// 230:        INTA230
	231,	// 231:        INTA231
	232,	// 232:        INTA232
	233,	// 233:        INTA233
	234,	// 234:        INTA234
	235,	// 235:        INTA235
	236,	// 236:        INTA236
	237,	// 237:        INTA237
	238,	// 238:        INTA238
	239,	// 239:        INTA239
	240,	// 240:        INTA240
	241,	// 241:        INTA241
	242,	// 242:        INTA242
	243,	// 243:        INTA243
	244,	// 244:        INTA244
	245,	// 245:        INTA245
	246,	// 246:        INTA246
	247,	// 247:        INTA247
	248,	// 248:        INTA248
	249,	// 249:        INTA249
	250,	// 250:        INTA250
	251,	// 251:        INTA251
	252,	// 252:        INTA252
	253,	// 253:        INTA253
	254,	// 254:        INTA254
	255,	// 255:        INTA255
};

#endif /* CPU_R5F565N */