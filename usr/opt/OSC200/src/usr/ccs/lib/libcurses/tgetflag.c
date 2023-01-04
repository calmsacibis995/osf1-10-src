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
static char rcsid[] = "@(#)$RCSfile: tgetflag.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/06/12 22:39:31 $";
#endif
/*
 * HISTORY
 */
/*** "tgetflag.c  1.6  com/lib/curses,3.1,9008 12/14/89 17:52:54"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   tgetflag
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */
/*
 *	NOTE:
 * 		This module is not actually built in to the library
 * 		Is is generated from termcap.form.
 *		See the Makefile and termcap.ed.
 */
/* Modification History
 *
 * 001 gws
 *	changed tgetflag() case "ns" to return rv=false if terminfo
 *	"scroll_forward" field is non-empty, instead of only if
 *	"scroll-forward" was set to Ctrl-j (10).  The old code incorrectly
 *	assumpted that Ctrl-j is the only control code to causes forward
 *	scrolling at the bottom of a screen.
 *
 */

#include "cursesext.h"

#define two(s1, s2) (s1 + 256*s2)
#define twostr(str) two(*str, str[1])

/*
 * NAME:        tgetflag
 *
 * FUNCTION:
 *
 *      Simulation of termcap using terminfo.
 */

int
tgetflag(id)
char *id;
{
	register int rv;
	register char *p;

	switch (twostr(id)) {
	case two('b','w'): rv = auto_left_margin; break;
	case two('a','m'): rv = auto_right_margin; break;
	case two('x','b'): rv = beehive_glitch; break;
	case two('x','s'): rv = ceol_standout_glitch; break;
	case two('x','n'): rv = eat_newline_glitch; break;
	case two('e','o'): rv = erase_overstrike; break;
	case two('g','n'): rv = generic_type; break;
	case two('h','c'): rv = hard_copy; break;
	case two('k','m'): rv = has_meta_key; break;
	case two('h','s'): rv = has_status_line; break;
	case two('i','n'): rv = insert_null_glitch; break;
	case two('d','a'): rv = memory_above; break;
	case two('d','b'): rv = memory_below; break;
	case two('m','i'): rv = move_insert_mode; break;
	case two('m','s'): rv = move_standout_mode; break;
	case two('o','s'): rv = over_strike; break;
	case two('e','s'): rv = status_line_esc_ok; break;
	case two('x','t'): rv = teleray_glitch; break;
	case two('h','z'): rv = tilde_glitch; break;
	case two('u','l'): rv = transparent_underline; break;
	case two('x','o'): rv = xon_xoff; break;
	case two('b','s'):
		p = cursor_left;
		rv = p && *p==8 && p[1] == 0;
		break;
	case two('p','t'):
		p = tab;
		rv = p && *p==9 && p[1] == 0;
		break;
	case two('n','c'):
		p = carriage_return;
		rv = ! (p && *p==13 && p[1] == 0);
		break;
	case two('n','s'):
		p = scroll_forward;
		rv = ! (p && p[0] != 0);		/* 001 */
		break;
	default: rv = 0;
	}
	return rv;
}
