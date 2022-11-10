/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 */

/*
 *	dev_rtc.h
 */
#ifndef __DEV_RTC_H__
#define __DEV_RTC_H__

#include <sys/machine.h>

#define RTC_PATH_(a)		#a
#define RTC_PATH(a)		RTC_PATH_(a)
#define RTC_SYSDEP()		RTC_PATH(../rtc/sysdepend/TARGET_DIR/dev_rtc.h)
#include RTC_SYSDEP()

#define	RTC_DEVNM	"CLOCK"

/* CLOCK data numbers */
typedef enum {
	/* Common attributes */
	DN_CKEVENT = TDN_EVENT,
	/* Device-specific attributes */
	DN_CKDATETIME = -100,
	DN_CKAUTOPWON = -101,
	/* Hardware-dependent functions */
	DN_CKREGISTER = -200
} ClockDataNo;

/* Set/get current time */
typedef struct {
	W d_year;		/* offset from 1900 */
	W d_month;		/* month(1 to 12) */
	W d_day;		/* day(1 to 31) */
	W d_hour;		/* hour(0 to 23) */
	W d_min;		/* minute(0 to 59) */
	W d_sec;		/* second(0 to 59) */
	W d_week;		/* week of year(not use) */
	W d_wday;		/* day of week(0 to 6, 0 is SUN) */
	W d_days;		/* day of year(not use) */
} DATE_TIM;

/* Read/write in non-volatile registers */
typedef struct {
	W nreg; 		/* number of the registers */
	struct ck_reg {
		W regno;	/* register number */
		UW data;	/* data for the register */
	} c[1];
} CK_REGS;

typedef	T_DEVEVT_ID	ClockEvt;

IMPORT	ER	rtcDrvEntry(void);
IMPORT	ID	CLOCK_mbfid;

#endif /* __DEV_RTC_H__ */