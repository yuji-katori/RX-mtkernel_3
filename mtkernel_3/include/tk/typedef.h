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

/*
 *	typedef.h
 *
 *	T-Kernel Standard Data Type Definition
 */

#ifndef	__TK_TYPEDEF_H__
#define __TK_TYPEDEF_H__

#include <sys/machine.h>

#ifdef CHK_TKERNEL_CONST
#define CONST	const
#else
#define CONST
#endif

/*
 * General-purpose data type  
 */
typedef signed char		B;		/* Signed 8 bit integer */
typedef signed short		H;		/* Signed 16 bit integer */
typedef signed long		W;		/* Signed 32 bit integer */
typedef signed long long	D;		/* Signed 64 bit integer */
typedef unsigned char		UB;		/* Unsigned 8 bit integer */
typedef unsigned short  	UH;		/* Unsigned 16 bit integer */
typedef unsigned long		UW;		/* Unsigned 32 bit integer */
typedef unsigned long long	UD;		/* Unsigned 64 bit integer */

typedef char			VB;		/* Nonuniform type 8 bit data */
typedef signed short		VH;		/* Nonuniform type 16 bit data */
typedef signed long		VW;		/* Nonuniform type 32 bit data */
typedef signed long long	VD;		/* Nonuniform type 64 bit data */

typedef volatile B		_B;		/* Volatile statement attached */
typedef volatile H		_H;
typedef volatile W		_W;
typedef volatile D		_D;
typedef volatile UB		_UB;
typedef volatile UH		_UH;
typedef volatile UW		_UW;
typedef volatile UD		_UD;

typedef signed int		INT;		/* Processor bit width signed integer */
typedef unsigned int		UINT;		/* Processor bit width unsigned integer */

typedef INT			SZ;		/* Size general */

typedef INT			ID;		/* ID general */
typedef	W			MSEC;		/* Time general (millisecond) */

/*
 * Data type in which meaning is defined in T-Kernel/OS specification 
 */
typedef INT			FN;		/* Function code */
typedef UW			ATR;		/* Object/handler attribute */
typedef INT			ER;		/* Error code */
typedef INT			PRI;		/* Priority */
typedef W			TMO;		/* Time out setting */
typedef UW			RELTIM;		/* Relative time */

typedef struct systim {				/* System time */
	W			hi;		/* Upper 32 bits */
	UW			lo;		/* Lower 32 bits */
} SYSTIM;

typedef D			SYSTIM_U;	/* System time (64bit) */

#ifdef CLANGSPEC
typedef void			(*TSKFP)(INT,void *);	/* Task Function address general */
typedef void			(*TIMFP)(void *);	/* Timevent Function address general */
typedef void			(*INTFP)(UINT);		/* Interrupt Function address general */
typedef INT			(*SVCFP)(void *,FN);	/* SVC Handler address general */
#endif /* CLANGSPEC */
typedef void			(*FP)();	/* Function address general */
typedef INT			(*FUNCP)();	/* Function address general */

#define LOCAL			static		/* Local symbol definition */
#define EXPORT					/* Global symbol definition */
#define IMPORT			extern		/* Global symbol reference */

/*
 * Boolean value 
 *	Defined as TRUE = 1, but it is always true when not 0.
 *	Thus, comparison such as bool = TRUE are not permitted.
 *	Should be as per bool !=FALSE.
 */
typedef UINT			BOOL;
#define TRUE			1		/* True */
#define FALSE			0		/* False */

/*
 * Common constant
 */
#ifndef NULL
#define NULL		0
#endif

#define TA_NULL		0U		/* No special attributes indicated */
#define TMO_POL		0		/* Polling */
#define TMO_FEVR	(-1)		/* Permanent wait */

/* ------------------------------------------------------------------------ */

#endif /* __TK_TYPEDEF_H__ */
