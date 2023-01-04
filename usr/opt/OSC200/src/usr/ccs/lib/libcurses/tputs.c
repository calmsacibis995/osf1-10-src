/* 
 * *****************************************************************
 * *                                                               *
 * *    Copyright (c) Digital Equipment Corporation, 1991, 1994    *
 * *                                                               *
 * *   All Rights Reserved.  Unpublished rights  reserved  under   *
 * *   the copyright laws of the United States.                    *
 * *                                                               *
 * *   The software contained on this media  is  proprietary  to   *
 * *   and  embodies  the  confidential  technology  of  Digital   *
 * *   Equipment Corporation.  Possession, use,  duplication  or   *
 * *   dissemination of the software and media is authorized only  *
 * *   pursuant to a valid written license from Digital Equipment  *
 * *   Corporation.                                                *
 * *                                                               *
 * *   RESTRICTED RIGHTS LEGEND   Use, duplication, or disclosure  *
 * *   by the U.S. Government is subject to restrictions  as  set  *
 * *   forth in Subparagraph (c)(1)(ii)  of  DFARS  252.227-7013,  *
 * *   or  in  FAR 52.227-19, as applicable.                       *
 * *                                                               *
 * *****************************************************************
 */
/*
 * (c) Copyright 1990, 1991, 1992, 1993 OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: tputs.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/12 22:46:59 $";
#endif
/*
 * HISTORY
 */
/*** "tputs.c  1.11  com/lib/curses,3.1,9008 1/30/90 19:54:27"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   tputs, _tpad
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1979 Regents of the University of California.
 */

#include <ctype.h>
#include "curses.h"
#include "term.h"
#ifdef NONSTANDARD
# include "ns_curses.h"
#endif

/*
 * NAME:        tputs
 *
 * FUNCTION:
 *
 *      Put the character string cp out, with padding.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The number of affected lines is affcnt, and the routine
 *      used to output one character is outc.
 */

tputs(cp, affcnt, outc)
	register char *cp;
	int affcnt;
	int (*outc)();
{
	char *_tpad();
					/* support non-VTD only.        */
	if (cp == NULL || *cp == (char)(0x80) ||
		*cp == (char)(0x00))
		return;

	/*
	 * The guts of the string.
	 */
	while (*cp)
		if (*cp == '$' && cp[1] == '<')
			cp = _tpad(cp, affcnt, outc);
		else
			(*outc)(*cp++);
}

/*
 * NAME:        _tpad
 */

static char *
_tpad(cp, affcnt, outc)
	register char *cp;
	int affcnt;
	int (*outc)();
{
	register int delay = 0;
	register char *icp = cp;

	/* Eat initial $< */
	cp += 2;

	/*
	 * Convert the number representing the delay.
	 */
	if (isdigit(*cp)) {
		do
			delay = delay * 10 + *cp++ - '0';
		while (isdigit(*cp));
	}
	delay *= 10;
	if (*cp == '.') {
		cp++;
		if (isdigit(*cp))
			delay += *cp - '0';
		/*
		 * Only one digit to the right of the decimal point.
		 */
		while (isdigit(*cp))
			cp++;
	}

	/*
	 * If the delay is followed by a `*', then
	 * multiply by the affected lines count.
	 */
	if (*cp == '*')
		cp++, delay *= affcnt;
	if (*cp == '>')
		cp++;	/* Eat trailing '>' */
	else {
		/*
		 * We got a "$<" with no ">".  This is usually caused by
		 * a cursor addressing sequence that happened to generate
		 * $<.  To avoid an infinite loop, we output the $ here
		 * and pass back the rest.
		 */
		(*outc)(*icp++);
		return icp;
	}

	/*
	 * If no delay needed, or output speed is
	 * not comprehensible, then don't try to delay.
	 */
	if (delay == 0)
		return cp;
	/*
	 * Let handshaking take care of it - no extra cpu load from pads.
	 * Also, this will be more optimal since the pad info is usually
	 * worst case.  We only use padding info for such terminals to
	 * estimate the cost of a capability in choosing the cheapest one.
	 */
	if (xon_xoff)
		return cp;
	(void) _delay(delay, outc);
	return cp;
}
