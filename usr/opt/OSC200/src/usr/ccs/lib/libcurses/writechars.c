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
static char rcsid[] = "@(#)$RCSfile: writechars.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 23:03:31 $";
#endif

/*
 * HISTORY
 */
/*
 */ 
/*** "writechars.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:48:27"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _writechars
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include "cursesext.h"

#ifdef WCHAR
#include        <locale.h>
#include        <wchar.h>
#include        <stdlib.h>
#endif

char *tparm();

extern	int	_outch();

/*
 * NAME:        _writechars
 */

_writechars (start, end)
#ifdef WCHAR
chtype *start, *end;
#else
register char	*start, *end; 
#endif
{
	register int c;
#ifdef WCHAR
	chtype	*p;
	int	len;
#else
	register char *p;
#endif
	extern int didntdobotright;	/* did not output char in corner */

#ifdef DEBUG
#ifdef WCHAR
	chtype	*chp;
	if(outf) fprintf(outf, "_writechars(%d:'", end-start+1);
	for( chp = start; chp <= end; chp++ ) 
		fprintf( outf, "0x%03x(%wc)", *chp );
	if(outf) fprintf(outf, "').\n");
#else
	if(outf) fprintf(outf, "_writechars(%d:'", end-start+1);
	if(outf) fwrite(start, sizeof (char), end-start+1, outf);
	if(outf) fprintf(outf, "').\n");
#endif
#endif	/* DEBUG */
	_setmode ();
	_sethl();
	while( start <= end )
	{
#ifdef FULLDEBUG
	if(outf) fprintf(outf,
	"wc loop: repeat_char '%s', SP->phys_irm %d, *start '%c'\n",
	repeat_char, SP->phys_irm, *start);
#endif  /* FULLDEBUG */
#ifdef WCHAR
		if (repeat_char && SP->phys_irm != 1 && (end-start) > 4 &&
#else
		if (repeat_char && SP->phys_irm != 1 &&
#endif
			((p=start+1),*start==*p++) && (*start==*p++) &&
			(*start==*p++) && (*start==*p++) && p<=end) {
			/* We have a run of at least 5 characters */
			c = 5;
			while (p <= end && *start == *p++)
				c++;
			SP->phys_x += c;
			/* Don't assume anything about how repeat and auto
			 * margins interact.  The concept is different. */
			while (SP->phys_x >= columns-1 && auto_right_margin) {
				c--; p--; SP->phys_x--;
			}
#ifdef DEBUG
			if(outf) fprintf(outf,
				"using repeat, count %d, char '%c'\n",
				c, *start);
#endif	/* DEBUG */
			tputs(tparm(repeat_char, *start, c), c, _outch);
			start = p-1;
			if (*start == start[-1])
				start++;
#ifdef PHASE2
			SP->curatr += c;
#endif
			continue;
		}
		c = *start++;
#ifdef DEBUG
#ifdef WCHAR
		if (outf) fprintf(outf,
			"c is '%x', phys_x %d, phys_y %d\n",
			c, SP->phys_x, SP->phys_y);
#else
		if (outf) fprintf(outf,
			"c is '%c', phys_x %d, phys_y %d\n",
			c, SP->phys_x, SP->phys_y);
#endif
#endif  /* DEBUG */
		if(SP->phys_irm == 1 && insert_character)
		{
			tputs(insert_character, columns-SP->phys_x, _outch);
		}
		/*
		 * If transparent_underline && !erase_overstrike,
		 * should probably do clr_eol.  No such terminal yet.
		 */
		if(transparent_underline && erase_overstrike &&
		   c == '_' && SP->phys_irm != 1)
		{
			_outch (' ');
			tputs(cursor_left, 1, _outch);
		}			
		if( ++SP->phys_x >= columns && auto_right_margin )
		{
					/* Have to on c100 anyway..*/
			if( SP->phys_y >= lines-1 /*&& !eat_newline_glitch*/ )
			{
				/*
				 * We attempted to put something in the last
				 * position of the last line.  Since this will
				 * cause a scroll (we only get here if the
				 * terminal has auto_right_margin) we refuse
				 * to put it out.
				 */
#ifdef DEBUG
				if(outf) fprintf(outf,
					"Avoiding lower right corner\n");
#endif
				didntdobotright = 1;
				SP->phys_x--;
#ifdef WCHAR
				return -1;
#else
				return;
#endif
			}
			SP->phys_x = 0;
			SP->phys_y++;
		}
		if( tilde_glitch && c == '~' )
		{
			_outch('`');
		}
		else
		{
#ifdef PHASE2
			_w_outch( c );
			SP->curatr++;
#else
			_outch( c );
#endif
		}
		/* Only 1 line can be affected by insert char here */
		if( SP->phys_irm == 1 && insert_padding )
		{
			tputs(insert_padding, 1, _outch);
		}
		if( eat_newline_glitch && SP->phys_x == 0 )
		{
			/*
			 * This handles both C100 and VT100, which are
			 * different.  We don't output carriage_return
			 * and cursor_down because it might confuse a
			 * terminal that is looking for return and linefeed.
			 */
			_outch('\r');
			_outch('\n');
		}
	}
}
