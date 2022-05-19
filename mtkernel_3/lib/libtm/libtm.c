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
 *    libtm.c
 *    T-Monitor compatible calls library
 */
#include <tk/tkernel.h>
#include <tm/tmonitor.h>
#include "libtm.h"

/*
 * libtm_init() - libtm Initialize
 * supported only on wait != 0 (polling not supported)
 */
EXPORT void libtm_init(void)
{
	tm_com_init();
}

/*
 * tm_getchar() - Get Character
 * supported only on wait != 0 (polling not supported)
 */
EXPORT INT tm_getchar( INT wait )
{
#ifdef CLANGSPEC
	VB	p;
#else
	UB	p;
#endif /* CLANGSPEC */
	UINT	imask;

	DI(imask);
	tm_rcv_dat(&p, 1);
	EI(imask);

	return p;
}

/*
 * tm_getline() - Get Line
 * special key is not supported
 */
#ifdef CLANGSPEC
EXPORT INT tm_getline( VB *buff )
{
	VB* p = buff;
#else
EXPORT INT tm_getline( UB *buff )
{
	UB* p = buff;
#endif /* CLANGSPEC */
	int len = 0;
	static const char LF = CHR_LF;
	INT imask;

	DI(imask);
	while (1) {
		tm_rcv_dat(p, 1);
		tm_snd_dat(p, 1);
		if (*p == CHR_CR) {
#ifdef CLANGSPEC
			tm_snd_dat(&LF, 1);
#else
			tm_snd_dat((const UB*)&LF, 1);
#endif /* CLANGSPEC */
			break;
		} else if (*p == CHR_ETX) {
			len = -1;
			break;
		}
		p++; len++;
	}
	*p = 0x00;
	EI(imask);

	return len;
}

/*
 * tm_putchar()
 * Ctrl-C is not supported
 */
EXPORT INT tm_putchar( INT c )
{
	static const char CR = CHR_CR;
#ifdef CLANGSPEC
	VB buf = c;
#else
	UB buf = (UB)c;
#endif /* CLANGSPEC */
	INT imask;

	DI(imask);
	if (buf == CHR_LF) {
#ifdef CLANGSPEC
		tm_snd_dat(&CR, 1);
#else
		tm_snd_dat((const UB*)&CR, 1);
#endif /* CLANGSPEC */
	}
	tm_snd_dat(&buf, 1);
	EI(imask);

	return 0;
}

/*
 * tm_putstring() - Put String
 * Ctrl-C is not supported
 */
#ifdef CLANGSPEC
EXPORT INT tm_putstring( const VB *buff )
{
	const VB* p = buff;
#else
EXPORT INT tm_putstring( const UB *buff )
{
	const UB* p = buff;
#endif /* CLANGSPEC */
	INT imask;

	DI(imask);
#ifdef CLANGSPEC
	while ( *p != '\0' ) {
#else
	while ( *p != (UB)'\0' ) {
#endif /* CLANGSPEC */
		tm_putchar(*p++);
	}
	EI(imask);

	return 0;
}
