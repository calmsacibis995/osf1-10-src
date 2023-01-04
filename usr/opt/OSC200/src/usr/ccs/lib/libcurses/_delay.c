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
static char rcsid[] = "@(#)$RCSfile: _delay.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 20:31:09 $";
#endif
/*
 * HISTORY
 */
/***
 ***  "_delay.c	1.6  com/lib/curses,3.1,8943 10/16/89 22:54:32";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _delay
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include <ctype.h>
#include "curses.h"
#include "term.h"
#ifdef NONSTANDARD
# include "ns_curses.h"
#endif

/*
 * The following array gives the number of tens of milliseconds per
 * character for each speed as returned by gtty.  Thus since 300
 * baud returns a 7, there are 33.3 milliseconds per char at 300 baud.
 */
static
_SHORT	tmspc10[] = {
	/* 0   50    75   110 134.5 150  200  300   baud */
	   0, 2000, 1333, 909, 743, 666, 500, 333,
	/* 600 1200 1800 2400 4800 9600 19200 38400 baud */
	   166, 83,  55,  41,  20,  10,   5,    2
};

/*
 * NAME:        _delay
 *
 * FUNCTION:
 *
 *      Insert a delay into the output stream for "delay/10" milliseconds.
 *      Round up by a half a character frame, and then do the delay.
 *      Too bad there are no user program accessible programmed delays.
 *      Transmitting pad characters slows many terminals down and also
 *      loads the system.
 */

_delay(delay, outc)
register int delay;
int (*outc)();
{
	register int mspc10;
	register int pc;
	register int outspeed;

#ifndef 	NONSTANDARD
# ifdef USG
	outspeed = cur_term->Nttyb.c_cflag&CBAUD;
# else
	outspeed = cur_term->Nttyb.sg_ospeed;
# endif
#else		NONSTANDARD
	outspeed = outputspeed(cur_term);
#endif		/* NONSTANDARD */
	if (outspeed <= 0 || outspeed >=
				(sizeof tmspc10 / sizeof tmspc10[0]))
		return ERR;

	mspc10 = tmspc10[outspeed];
	delay += mspc10 / 2;
	if (pad_char)
		pc = *pad_char;
	else
		pc = 0;
	for (delay /= mspc10; delay > 0; delay--)
		(*outc)(pc);
	return OK;
}
