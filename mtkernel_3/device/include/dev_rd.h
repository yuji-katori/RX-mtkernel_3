/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2022/11/18.
 *----------------------------------------------------------------------
 */

/*
 *	dev_rd.h
 */
#ifndef __DEV_RD_H__
#define __DEV_RD_H__

#include <dev_disk.h>
#include <sys/machine.h>

#define RD_PATH_(a)		#a
#define RD_PATH(a)		RD_PATH_(a)
#define RD_SYSDEP()		RD_PATH(../rd/sysdepend/TARGET_DIR/dev_rd.h)
#include RD_SYSDEP()

#endif /* __DEV_RD_H__ */