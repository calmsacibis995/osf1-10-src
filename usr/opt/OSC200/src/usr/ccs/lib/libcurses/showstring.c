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
static char rcsid[] = "@(#)$RCSfile: showstring.c,v $ $Revision: 4.2.4.3 $ (DEC) $Date: 1993/09/23 18:30:29 $";
#endif
/*
 * HISTORY
 */
/*** 1.10.1.1  com/lib/curses/showstring.c, bos, bos320 3/4/91 18:29:36 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _showstring
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
#include "libcurses_msg.h"

static nl_catd catd;

/*
 * NAME:        _showstring
 *
 * FUNCTION:
 *
 *      Dump the string running from first to last out to the terminal.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Take into account attributes, and attempt to take advantage of
 *      large pieces of white space and text that's already there.
 *      oldline is the old text of the line.
 *
 *      Variable naming convention: *x means "extension", e.g. a rubber band
 *      that briefly looks ahead; *c means a character version of an
 *      otherwise chtype pointer; old means what was on the screen before
 *      this call; left means the char 1 space to the left.
 */

_showstring(sline, scol, first, last, oldlp)
int sline, scol;
chtype *first, *last; 
struct line *oldlp;
{
	register int hl = 0;    /* nontrivial line, highlighted/with holes*/
	int prevhl=SP->virt_gr, thishl;	/* highlight state tty is in	 */
	register chtype *p, *px;	/* current char being considered */
	register chtype *oldp, *oldpx;	/* stuff under p and px		 */
#ifdef WCHAR
	register chtype *pc, *pcx;	/* like p, px but in char buffer */
#else
	register char *pc, *pcx;	/* like p, px but in char buffer */
#endif
	chtype *tailoldp;		/* last valid oldp		 */
	int oldlen;			/* length of old line		 */
	int lcol, lrow;			/* position on screen		 */
	int oldc;			/* char at oldp			 */
	int leftoldc, leftnewc;		/* old & new chars to left of p  */
	int diff_cookies;		/* magic cookies changed	 */
	int diff_attrs;			/* highlights changed		 */
	chtype *oldline;
#ifdef NONSTANDARD
	static
#endif /* NONSTANDARD */
#ifdef WCHAR
	   chtype firstc[256], *lastc;	/* char copy of input first, last */
#else
	   char firstc[256], *lastc;	/* char copy of input first, last */
#endif
#ifdef PHASE2
	chtype	*p_atr;
#endif

#ifdef DEBUG
	if(outf) fprintf(outf,
		"_showstring((%d,%d) %d:'", sline, scol, last-first+1);
	if(outf)
		for (p=first; p<=last; p++) {
#ifdef WCHAR
			_sputc( *p, outf );
#else
			thishl = *p & A_ATTRIBUTES;
			if (thishl)
				putc('\'', outf);
			putc(*p & A_CHARTEXT, outf);
#endif
		}
	if(outf) fprintf(outf, "').\n");
#endif
	if (last-first > columns) {
		_pos(lines-1, 0);
#ifndef		NONSTANDARD
		catd = catopen(MF_LIBCURSES, NL_CAT_LOCALE);
		fprintf(stderr, catgets(catd, MS_SHOWSTRING, M_MSG_4,
		    "Bad call to _showstring, first %x, last %x, diff %dpcx\n"),
		    first, last, last-first);
		catclose(catd);
#endif
		abort();
	}
	if (oldlp) {
		oldline = oldlp->body;
		oldp = oldline+scol;
	}
	else
		oldp = 0;
	for (p=first,lastc=firstc; p<=last; ) {
		if (*p & A_ATTRIBUTES)
			hl++;	/* attributes on the line */
		if (oldp && (*oldp++ & A_ATTRIBUTES))
			hl++;	/* attributes on old line */
#ifdef WCHAR
		if ( (last - p) > 3 && ( *p==' ' && (px=p+1,*px++==' ')
			&& *px++==' ' && *px==' ') )
#else
		if (*p==' ' && (px=p+1,*px++==' ') && *px++==' ' && *px==' ')
#endif
			hl++;	/* a run of at least 4 blanks */
		*lastc++ = *p & A_CHARTEXT;
		p++;
#ifdef FULLDEBUG
	if(outf) fprintf(outf,
		"p %x '%c' %o, lastc %x %o, oldp %x %o, hl %d\n",
		p, p[-1], p[-1], lastc, lastc[-1], oldp,
		oldp ? oldp[-1] : 0, hl);
#endif
	}
	lastc--;

	lcol = scol; lrow = sline;
	if (oldlp) {
		oldline = oldlp->body;
		oldlen = oldlp->length;
		/* Check for runs of stuff that's already there. */
		for (p=first,oldp=oldline+lcol; p<=last; p++,oldp++) {
#ifdef WCHAR
			if( (last - p) > 3 && (*p==*oldp &&
				(px=p+1,oldpx=oldp+1,*px++==*oldpx++)
					  && *px++==*oldpx++ && *px==*oldpx) )

#else
			if (*p==*oldp &&
				(px=p+1,oldpx=oldp+1,*px++==*oldpx++)
					  && *px++==*oldpx++ && *px==*oldpx)
#endif
				hl++;	/* a run of at least 4 matches */
#ifdef FULLDEBUG
	if(outf) fprintf(outf,
	"p %x '%c%c%c%c', oldp %x '%c%c%c%c', hl %d\n",
	p, p[0], p[1], p[2], p[3],
	oldp, oldp[0], oldp[1], oldp[2], oldp[3],
	hl);
#endif
		}
	} else {
		oldline = NULL;
		oldlen = 0;
	}

	if (!hl) {
		/* Simple, common case.  Do it fast. */
		_pos(lrow, lcol);
		_hlmode(0);
		_writechars(firstc, lastc);
		return;
	}

#ifdef DEBUG
	if(outf) fprintf(outf,
		"oldlp %x, oldline %x, oldlen %d 0x%x\n",
		oldlp, oldline, oldlen, oldlen);
	if(outf) fprintf(outf, "old body('");
	if (oldlp)
		for (p=oldline; p<=oldline+oldlen; p++)
			if(outf) fprintf(outf, "%c", *p);
	if(outf) fprintf(outf, "').\n");
#endif

#ifdef WCHAR
	if( lcol > 0 )
		oldc = first[-1];
	else
		oldc = ' ';       
#else
	oldc = first[-1];
#endif
	tailoldp = oldline + oldlen;
	for (p=first, oldp=oldline+lcol, pc=firstc; pc<=lastc;
						p++,oldp++,pc++) {
		thishl = *p & A_ATTRIBUTES;
#ifdef DEBUG
		if(outf) fprintf(outf,
			"prevhl %o, thishl %o\n", prevhl, thishl);
#endif
		leftoldc = oldc & A_ATTRIBUTES;
#ifdef WCHAR
		if( lcol > 0 )
			leftnewc = p[-1] & A_ATTRIBUTES;
		else
			leftnewc = 0;
#else
		leftnewc = p[-1] & A_ATTRIBUTES;
#endif
		diff_cookies = (magic_cookie_glitch>=0) &&
					(leftoldc != leftnewc);
		diff_attrs = ceol_standout_glitch &&
					(thishl != leftnewc);
		if (oldp >= tailoldp)
			oldc = ' ';
		else
			oldc = *oldp;
#ifdef FULLDEBUG
		if(outf) fprintf(outf,
"p %x *p %o, pc %x *pc %o, oldp %x, *oldp %o, lcol %d, lrow %d, oldc %o\n",
p, *p, pc, *pc, oldp, *oldp, lcol, lrow, oldc);
#endif
		if (*p != oldc || SP->virt_irm == 1 || diff_cookies ||
			diff_attrs || insert_null_glitch &&
			(oldp >= oldline+oldlen)) {
			register int n;

			_pos(lrow, lcol);

			/*
			 * HP 2645/2626: output new for each char.
			 * This forces it to be right no matter what
			 * was there before.
			 */
			if (ceol_standout_glitch && thishl == 0 &&
							oldc&A_ATTRIBUTES) {
#ifdef FULLDEBUG
				if(outf) fprintf(outf,
					"ceol %d, thishl %d, prevhl %d\n",
					ceol_standout_glitch, thishl, prevhl);
#endif
				_forcehl();
			}

			/* Force highlighting to be right */
			_hlmode(thishl);
			if (thishl != prevhl) {
				if (magic_cookie_glitch >= 0) {
					_sethl();
					p += magic_cookie_glitch;
					oldp += magic_cookie_glitch;
					pc += magic_cookie_glitch;
					lcol += magic_cookie_glitch;
				}
			}

			/*
			 * Gather chunks of chars together, to be more
			 * efficient, and to allow repeats to be detected.
			 * Not done for blanks on cookie terminals because
			 * the last one might be a cookie.
			 */
			if (magic_cookie_glitch<0 || *pc != ' ') {
				for (px=p+1,oldpx=oldp+1;
					px<=last && *p==*px;
					px++,oldpx++) {
					if(!(repeat_char && oldpx<tailoldp &&
							*p==*oldpx))
						break;
				}
				px--; oldpx--;
				n = px - p;
				pcx = pc + n;
			} else {
				n = 0;
				pcx = pc;
			}
#ifdef WCHAR
#ifdef PHASE2
			p_atr = SP->curatr + n;
			while( pcx < lastc && IS_NEXTATR( *++p_atr ) )
				pcx++, n++;
			SP->curatr += (n + 1);
#else
			while( pcx < lastc && IS_NEXTCHAR( pcx[1] ) )
				pcx++, n++;
#endif
#endif
			_writechars(pc, pcx);
			lcol += n; pc += n; p += n; oldp += n;
			prevhl = thishl;
		}
		lcol++;
	}
	if (magic_cookie_glitch >= 0 && prevhl) {
		/* Have to turn off highlighting at end of line */
		_hlmode(0);
		_sethl();
	}
}
