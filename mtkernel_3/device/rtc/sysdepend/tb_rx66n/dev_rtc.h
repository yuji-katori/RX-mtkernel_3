/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.2.
 *----------------------------------------------------------------------
 */

/*
 *	dev_rtc.h
 */

#define RTC_OPEN( )
#define RTC_CLOSE( )
#define RTC_READ( )	dt->d_year  = 100;		\
			dt->d_month =   1;		\
			dt->d_day   =   1;		\
			dt->d_hour  =   0;		\
			dt->d_min   =   0;		\
			dt->d_sec   =   0;

#define RTC_WRITE( )
#define	RTC_INIT( )