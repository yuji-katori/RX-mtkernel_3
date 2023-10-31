/*
 *----------------------------------------------------------------------
 *    micro T-Kernel 3.00.00
 *
 *    Copyright (C) 2022 by Yuji Katori.
 *    This software is distributed under the T-License 2.1.
 *----------------------------------------------------------------------
 *    Modified by Yuji Katori at 2023/10/23.
 *----------------------------------------------------------------------
 */

/*
 *	r_ether_rx_config.h
 */
#include "config/config_tcpudp.h"
#include "sys/machine.h"
#define ETHER_PATH_(a)		#a
#define ETHER_PATH(a)		ETHER_PATH_(a)
#define ETHER_SYSDEP()		ETHER_PATH(sysdepend/TARGET_DIR/ether_config.h)
#include ETHER_SYSDEP()