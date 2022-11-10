/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	drv_rtc.h
 */

#include "iodefine.h"

#define	BCDtoDEC( x )	((x) / 16 * 10 + (x) % 16)
#define	DECtoBCD( x )	((x) / 10 * 16 + (x) % 10)

#define RTC_OPEN( )	RTC.RCR4.BYTE = 0x00;		/* Select Sub Clock	*/\
			RTC.RCR2.BIT.START = 0;		/* Stop RTC		*/\
			while( RTC.RCR2.BIT.START )  ;	/* Wait RTC Stop	*/\
			RTC.RCR2.BIT.RESET = 1;		/* Reset RTC		*/\
			while( RTC.RCR2.BIT.RESET )  ;	/* Wait RTC Reset	*/\
			RTC.RCR2.BIT.HR24 = 1;		/* 24H Mode		*/\
			RTC.RYRCNT.WORD  = 0;		/* 1980			*/\
			RTC.RMONCNT.BYTE = 1;		/* 1			*/\
			RTC.RDAYCNT.BYTE = 1;		/* 1			*/\
			RTC.RWKCNT.BYTE  = 2;		/* 0 is SUN		*/\
			RTC.RHRCNT.BYTE  = 0;		/* 0			*/\
			RTC.RMINCNT.BYTE = 0;		/* 0			*/\
			RTC.RWKCNT.BYTE  = 0;		/* 0			*/\
			RTC.RCR2.BIT.START = 1;		/* Start RTC		*/\
			while( !RTC.RCR2.BIT.START )  ;	/* Wait RTC Start	*/

#define RTC_CLOSE( )	RTC.RCR2.BIT.START = 0;		/* Stop RTC		*/

#define RTC_READ( )	dt->d_year  = BCDtoDEC( RTC.RYRCNT.WORD ) + 80;		\
			dt->d_month = BCDtoDEC( RTC.RMONCNT.BYTE );		\
			dt->d_day   = BCDtoDEC( RTC.RDAYCNT.BYTE );		\
			dt->d_hour  = BCDtoDEC( RTC.RHRCNT.BYTE );		\
			dt->d_min   = BCDtoDEC( RTC.RMINCNT.BYTE );		\
			dt->d_sec   = BCDtoDEC( RTC.RSECCNT.BYTE );		\
			dt->d_week  = 0;					\
			dt->d_wday  = RTC.RWKCNT.BYTE;				\
			dt->d_days  = 0;

#define RTC_WRITE( )	RTC.RYRCNT.WORD  = DECtoBCD( dt->d_year - 80 );		\
			RTC.RMONCNT.BYTE = DECtoBCD( dt->d_month );		\
			RTC.RDAYCNT.BYTE = DECtoBCD( dt->d_day );		\
			RTC.RWKCNT.BYTE  = dt->d_wday;				\
			RTC.RHRCNT.BYTE  = DECtoBCD( dt->d_hour );		\
			RTC.RMINCNT.BYTE = DECtoBCD( dt->d_min );		\
			RTC.RSECCNT.BYTE = DECtoBCD( dt->d_sec );

#define	RTC_INIT( )	tk_dis_dsp( );					/* Dispatch Disable			*/\
			SYSTEM.PRCR.WORD = 0xA503;			/* Protect Disable			*/\
			if( SYSTEM.SOSCCR.BYTE )  {			/* Sub-Clock Stoped ?			*/\
				SYSTEM.SOSCWTCR.BYTE = 33;		/* tSUBOSC is 2 seconds(Suitable).	*/\
				SYSTEM.SOSCCR.BYTE = 0x00;		/* Sub-Clock Stabilize			*/\
				while( !SYSTEM.OSCOVFSR.BIT.SOOVF )  ;	/* Wait Sub-Clock Stabilize		*/\
			}											  \
			SYSTEM.PRCR.WORD = 0xA500;			/* Protect Enable			*/\
			tk_ena_dsp( );					/* Dispatch Enable			*/
