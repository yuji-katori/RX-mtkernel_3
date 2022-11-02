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

#define	RTC_DEVNM	"CLOCK"

/* 属性データ */
/* CLOCKデータ番号 */
typedef enum {
	DN_CKDATETIME = (-100)
} ClockDataNo;

/* 現在時刻の設定／取得 */
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

IMPORT	ER	rtcDrvEntry( void );

#endif /* __DEV_RTC_H__ */