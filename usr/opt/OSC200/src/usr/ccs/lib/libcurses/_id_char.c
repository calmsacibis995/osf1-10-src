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
static char rcsid[] = "@(#)$RCSfile: _id_char.c,v $ $Revision: 4.2.4.2 $ (DEC) $Date: 1993/06/12 20:37:53 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "_id_char.c	1.6  com/lib/curses,3.1,8943 10/16/89 22:56:42";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _id_char, _ins_blanks, _ins_string
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

#define min(a,b) (a<b ? a : b)

static int _ins_string()   	/* predefine to remove warnings -- TEJ */;
static _ins_blanks()   		/* predefine to remove warnings -- TEJ */;

/*
 * NAME:        _id_char
 *
 * FUNCTION:
 *
 *      Modify current screen line 'old' to match desired line 'new'.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      The old line is at position ln.  Each line is divided into 4
 *      regions:
 *
 *      nlws, olws              - amount of leading white space on new/old
 *                                line
 *      com_head                - length of common head
 *      nchanged, ochanged      - length of the parts changed
 *      com_tail                - length of a common tail
 */

_id_char (old, new, ln)
register struct line   *old, * new;
{
	register chtype  *oc_beg, *nc_beg,/* Beginning of changed part */
			 *oc_end, *nc_end;/* End of changed part */
	chtype *p, *q;			/* scratch */
	int olws, nlws;			/* old/new leading white space */
	int com_head, com_tail;		/* size of common head/tail */
	int ochanged, nchanged;		/* size of changed part */
	int samelen;			/* both lines are same length */
	int nbl;	/* # blanks in new line replacing nonblanks in old */
	int nms;	/* # chars same in new and old in middle part */
	int cw_idc;	/* cost of update with insert/delete character */
	int cwo_idc;	/* cost of update without insert/delete character */
	int i, j, n;	/* scratch */
	int len, diff;
	static chtype nullline[] = {0, 0};
#ifdef WCHAR
	int delcnt = 0;
	chtype *nnn   ;
#endif
#ifdef PHASE2
	chtype	*oc_atr_end, *nc_atr_end;
#endif

#ifdef DEBUG
	if(outf) fprintf(outf, "_id_char(%x, %x, %d)\n", old, new, ln);
	if(outf) fprintf(outf, "old: ");
	if(outf) fprintf(outf, "%8x: ", old);
	if (old == NULL) {
		if(outf) fprintf(outf, "()\n");
	} else {
		if(outf) fprintf(outf, "%4d ", old->length);
		for (j=0; j<old->length; j++) {
#ifdef WCHAR
			_sputc( old->body[j], outf );
#ifdef PHASE2
			_sputc( old->body[j], outf );
#endif
#else
			n = old->body[j];
			if (n & A_ATTRIBUTES)
				putc('\'', outf);
			n &= 0177;
			if(outf) fprintf(outf, "%c", n>=' ' ? n : '.');
#endif
		}
		if(outf) fprintf(outf, "\n");
	}
	if(outf) fprintf(outf, "new: ");
	if(outf) fprintf(outf, "%8x: ", new);
	if (new == NULL) {
		if(outf) fprintf(outf, "()\n");
	} else {
		if(outf) fprintf(outf, "%4d ", new->length);
		for (j=0; j<new->length; j++) {
#ifdef WCHAR
			_sputc( new->body[j], outf );
#ifdef PHASE2
			_sputc( new->atr_body[j], outf );
#endif
#else
			n = new->body[j];
			if (n & A_ATTRIBUTES)
				putc('\'', outf);
			n &= 0177;
			if(outf) fprintf(outf, "%c", n>=' ' ? n : '.');
#endif
		}
		if(outf) fprintf(outf, "\n");
	}
#endif	/* DEBUG */

	if (old == new)
	{
		return;
	}

	/* Start at the ends of the lines */
	if( old )
	{
		oc_beg = old -> body;
		oc_end = &old -> body[old -> length];
	}
	else
	{
		oc_beg = nullline;
		oc_end = oc_beg;
	}
	if( new )
	{
		nc_beg = new -> body;
		nc_end = &new -> body[new -> length];
	}
	else
	{
		nc_beg = nullline;
		nc_end = nc_beg;
	}

	/* Find leading and trailing blanks */
	olws = nlws = com_head = com_tail = 0;
	while (*--oc_end == ' ' && oc_end >= oc_beg)
		;
	while (*--nc_end == ' ' && nc_end >= nc_beg)
		;
	samelen = (nc_end-nc_beg) == (oc_end-oc_beg)
		;
	while( *oc_beg == ' ' && oc_beg <= oc_end )
	{
		oc_beg++;
		olws++;
	}
	while( *nc_beg == ' ' && nc_beg <= nc_end )
	{
		nc_beg++;
		nlws++;
	}

	/*
	 * Now find common heads and tails (com_head & com_tail).  If the
	 * lengths are the same, the change was probably at the beginning,
	 * so count common tail first.  This only matters if it could match
	 * both ways, for example, when changing
	 * "                  ####"
	 *    to
	 * "####              ####"
	 */
	if( samelen )
	{
		while( *oc_end==*nc_end && oc_beg<=oc_end && nc_beg<=nc_end )
		{
			oc_end--;
			nc_end--;
			com_tail++;
		}
		while( *oc_beg==*nc_beg && oc_beg<=oc_end && nc_beg<=nc_end )
		{
			oc_beg++;
			nc_beg++;
			com_head++;
		}
	}
	else
	{
		while( *oc_beg==*nc_beg && oc_beg<=oc_end && nc_beg<=nc_end )
		{
			oc_beg++;
			nc_beg++;
			com_head++;
		}
		while( *oc_end==*nc_end && oc_beg<=oc_end && nc_beg<=nc_end )
		{
			oc_end--;
			nc_end--;
			com_tail++;
		}
	}
#ifdef	WCHAR
	/*
	 * Recount the trailing pad characters, if any
	 */
	while (IS_NEXTCHAR(oc_end[1]) && (com_tail > 0))
	{
		oc_end++   ;
		nc_end++   ;
		com_tail-- ;
	}
#endif	/* WCHAR */
	ochanged = oc_end - oc_beg + 1;
	nchanged = nc_end - nc_beg + 1;

	/* Optimization: lines are identical, so return now */
	if( ochanged==0 && nchanged==0 && nlws==olws )
	{
#ifdef DEBUG
		if(outf) fprintf(outf, "identical lines, returning early\n");
#endif
		return;
	}

#ifdef DEBUG
	if(outf) fprintf (outf, "Before costs: nlws=%2d  olws=%2d\
  com_head=%2d  nchanged=%2d  ochanged=%2d  com_tail=%2d, icfixed %d,\
 icvar %d/32\n", nlws, olws, com_head, nchanged, ochanged, com_tail,
		SP->term_costs.icfixed, SP->term_costs.icvar);
	if(outf) fprintf (outf, "samelen %d, *oc_beg %o *nc_beg %o\
 *oc_end %o *nc_end %o\n", samelen, *oc_beg, *nc_beg, *oc_end, *nc_end);
#endif /* DEBUG */

	/* Decide whether to use insert/delete character */
#define		costic(nchars)	(SP->term_costs.icfixed + \
	(((nchars)*SP->term_costs.icvar)>>5) )

	cw_idc = cwo_idc = nchanged;
	if( olws > nlws )				/* delete char */
	{
		cw_idc += costic(olws-nlws);
	}
	else
	{
		if( nlws > olws )			/* insert char */
		{
			cw_idc += costic(nlws-olws);
		}
	}
	if( ochanged > nchanged )			/* delete char */
	{
		cw_idc += costic(ochanged-nchanged);
	}
	else
	{
		if( nchanged > ochanged )		/* insert char */
		{
			cw_idc += costic(nchanged-ochanged);
		}
	}
	if( olws != nlws )
	{
		cwo_idc += com_head;
	}
	if( olws+ochanged != nlws+nchanged )
	{
		cwo_idc += com_tail;
	}

	if( cw_idc > cwo_idc )
	{
		/*
		 * It's cheaper to NOT use insert/delete character!
		 * This may be because the gain from saving the common head
		 * and tail is not worth the cost of doing the insert and
		 * delete chars (some terminals are slow at this, or the
		 * tails may be really short) or because the cost is
		 * infinite (i.e. the terminal does not have ins/del char).
		 * We note this fact by forgetting about the common heads
		 * and tails.
		 */
		ochanged += com_head+com_tail;
		nchanged += com_head+com_tail;
		oc_end += com_tail;
		nc_end += com_tail;
		oc_beg -= com_head;
		nc_beg -= com_head;
		com_head = 0;
		com_tail = 0;
	}

	/*
	 * On magic cookie terminals, we have to check for the possibility
	 * that there is a cookie that we must overwrite.  This is only
	 * necessary because a "go normal" cookie looks like an ordinary
	 * blank but must compare differently.
	 */
	if( magic_cookie_glitch >= 0 && com_tail > 0 && nc_end[1] == ' ' &&
			oc_end[0]&A_ATTRIBUTES && oc_end[1] == ' ' )
	{
#ifdef DEBUG
		if (outf) fprintf(outf,
			"adding one because of magic cookie\n");
#endif
		oc_end++;
		nc_end++;
		ochanged++;
		nchanged++;
		com_tail--;
	}

#ifdef DEBUG
	if(outf) fprintf (outf,
	"After cost: nlws=%2d olws=%2d com_head=%2d nchanged=%2d \
ochanged=%2d com_tail=%2d, cost w/idc %d, cost wo/idc %d\n",
	nlws, olws, com_head, nchanged, ochanged, com_tail, cw_idc, cwo_idc);
#endif /* DEBUG */

	/*
	 * Now actually go fix up the line.
	 * There are several cases to consider.
	 * The most important thing to keep in mind is
	 * that deletions need to be done before insertions
	 * to prevent shifting good text off the end of the line.
	 */

	if( com_head == 0 && com_tail == 0 )
	{
		/* No common text - must redraw entire line */
		if( ochanged == 0 && nchanged == 0 )
		{
			/* Empty lines - do nothing */
			_chk_typeahead();
			return;
		}
		/* If empty old line, pretend leading blanks */
		if( ochanged == 0 && !insert_null_glitch )
		{
			olws = nlws;
		}

		/* Make sure changed parts start at same column */
		j = nlws - olws;
		if( j > 0 )
		{
			nc_beg -= j;
			nchanged += j;
			nlws = olws;
		}
		else
		{
			if( j < 0 )
			{
				oc_beg += j;
				ochanged -= j;
				olws = nlws;
			}
		}

		for(nbl=nms=0,p=oc_beg,q=nc_beg; p<=oc_end && q<=nc_end;
								p++,q++)
		{
			if( *q==' ' && *p != ' ' )
			{
				nbl++;
			}
			if( *q == *p )
			{
				nms++;
			}
#ifdef FULLDEBUG
			if (outf) fprintf(outf,
			"*p '%c', *q '%c', nms %d, nbl %d\n",
			*p, *q, nms, nbl);
#endif
		}
		if( !clr_eol || insert_null_glitch || nms>=nbl )
		{
#ifdef PHASE2
			SP->curatr = &new->atr_body[min(nlws, olws)];
#endif
			_showstring(ln, min(nlws, olws),
						nc_beg, nc_end, old);
			if( nlws + nchanged < olws + ochanged )
			{
				_pos(ln, nlws + nchanged);
				_clreol();
			}
		}
		else
		{
			if( ochanged > 0 )
			{
				_pos(ln, olws);
				_clreol();
				for (p=oc_beg; p<=oc_end; p++)
					*p = ' ';
			}
#ifdef PHASE2
			SP->curatr = &new->atr_body[nlws];
#endif
			_showstring(ln, nlws, nc_beg, nc_end, old);
		}
		_chk_typeahead();
		return;
	}

	if( com_head==0 )
	{
		/* We have only a common tail. */
		if( nchanged == 0 && ochanged == 0 )
		{
			_chk_typeahead();
			return;
		}
		i = (nlws + nchanged) - (olws + ochanged);
		/* Simplify things - force olws == nlws */
		j = nlws - olws;
		if( j > 0 )
		{
			nc_beg -= j;
			nchanged += j;
			nlws = olws;
		}
		else
		{
			if( j < 0 )
			{
				oc_beg += j;
				ochanged -= j;
				olws = nlws;
			}
		}
		if( i >= 0 )
		{
#ifdef WCHAR
#ifdef PHASE2
			nnn = &new->atr_body[new->length]
				- (&new->body[new->length] - nc_end) - i;
			if( IS_NEXTATR( nnn[1] ) )
			{
				delcnt = get_pos( nnn[1] );
				i += get_pos( nnn[1] ); 
				_pos( ln, nlws+nchanged-i );
				_delchars( delcnt );
			}
			SP->curatr = &new->atr_body[nlws];
#else
			/*
			 * If nchanged > ochanged, it can happen that
			 * nc_end[-i+1] will point to a PAD char.
			 * In this case, the screen will not be update
			 * correctly. So we overwrite only complete character
			 * and calls _delchars to delete the extra characters.
			 */
			if ((i < nlws + nchanged) && IS_NEXTCHAR(*(nc_end-i+1))){
				while( IS_NEXTCHAR( *(nc_end-i) ) ){
					i++, delcnt++ ;
				}
				i++, delcnt++ ;
				_pos( ln, nlws+nchanged-i );
				_delchars( delcnt );
			}
#endif	
#endif
			_showstring(ln, nlws, nc_beg, nc_end-i, old);
			if( i > 0 )
			{
#ifdef PHASE2
				SP->curatr += (nchanged-i);
#endif
				_ins_string(ln, nlws+nchanged-i,
						nc_end-i+1, nc_end);
			}
		}
		else
		{
#ifdef PHASE2
			SP->curatr = &new->atr_body[nlws];
#endif
			_showstring(ln, nlws, nc_beg, nc_end, old);
			_delchars(-i);
		}
		_chk_typeahead();
		return;
	}

	/* At this point, we know there is a common head (com_head != 0) */
	if( nlws < olws )
	{
		/* Do the leading delete chars right away */
		_pos(ln, 0);
		_delchars(diff = olws - nlws);
		olws = nlws;
		len = old->length;
		for( i=0; i<len; i++ )
		{
			old->body[i] = old->body[i]+diff;
		}
	}
	if (com_tail == 0)
	{
		if( nchanged == 0 && ochanged == 0 )
		{
			if( nlws > olws )
			{
				_ins_blanks(ln, 0, nlws - olws);
			}
			_chk_typeahead();
			return;
		}
#ifdef PHASE2
		SP->curatr = &new->atr_body[olws+com_head];
#endif
		_showstring(ln, olws+com_head, nc_beg, nc_end, old);
		if( nchanged < ochanged )
		{
			_pos(ln, olws + com_head + nchanged);
			_clreol();
		}
		if( nlws > olws )
		{
			_ins_blanks(ln, 0, nlws - olws);
		}
	}
	else
	{
#ifdef WCHAR
		if( nchanged > 0 && ochanged > 0 )
		{
			i = min(nchanged, ochanged);
#ifdef PHASE2
			SP->curatr = &new->atr_body[olws+com_head];
#endif
			if (IS_NEXTCHAR(nc_beg[i]) && (i > 0)){
				while( IS_NEXTCHAR( *(nc_beg+i-1) ) ){
					i--, delcnt++ ;
				}
				i--, delcnt++ ;
			}
			_showstring(ln, olws+com_head, nc_beg,
						nc_beg+i-1, old);
		}
		if( nchanged < ochanged )
		{
#ifdef PHASE2
			SP->curatr = &new->atr_body[olws+com_head];
#endif
			_showstring(ln, olws+com_head, nc_beg,
					 nc_end, old);
			if( (i = ochanged - nchanged) > 0 )
			{	
				_pos(ln, nlws + com_head + nchanged );
				_delchars( i );
			}
		}
		else if( nchanged > ochanged )
		{
			if (delcnt > 0)
			{
				_pos(ln, olws+com_head+ochanged-delcnt) ;
				_delchars(delcnt) ;
				_ins_string(ln, olws+com_head+ochanged-delcnt,
					    nc_beg + ochanged - delcnt, nc_end);
			} else
				_ins_string(ln, olws+com_head+ochanged,
					    nc_beg + ochanged, nc_end);
		}
#else
		if( nchanged > 0 && ochanged > 0 )
		{
			i = min(nchanged, ochanged);
#ifdef PHASE2
			SP->curatr = &new->atr_body[olws+com_head];
#endif
			_showstring(ln, olws+com_head, nc_beg,
						nc_beg+i-1, old);
		}
		if( nchanged < ochanged )
		{
			_pos(ln, nlws + com_head + nchanged);
			_delchars(ochanged - nchanged);
		}
		else
		{
			if( nchanged > ochanged )
			{
#ifdef PHASE2
				SP->curatr += ochanged; 
#endif
				_ins_string(ln, olws+com_head+ochanged,
					nc_beg + ochanged, nc_end);
			}
		}
#endif
		if( nlws > olws )
		{
			_ins_blanks(ln, 0, nlws-olws);
		}
	}
	_chk_typeahead();
	return;
}

/*
 * NAME:        _ins_blanks
 *
 * FUNCTION:
 *
 *      Insert nblanks blanks at position (sline, scol).
 */

static int
_ins_blanks(sline, scol, nblanks)
int sline, scol, nblanks;
{
#ifdef DEBUG
	if (outf) fprintf(outf, "_ins_blanks at (%d, %d) %d blanks\n",
		sline, scol, nblanks);
#endif
	SP->virt_gr = 0;	/* 001 */
	_forcehl();		/* 001 */
	_pos(sline, scol);
	if( nblanks > 1 && parm_ich )
	{
		/* Insert the characters and then draw on the blanks */
		_inschars(nblanks);
	}
	else
	{
		/*
		 * Type the blanks in "insert mode".  This includes
		 * having to send insert_character before each character
		 * is output.
		 */
		_insmode(1);
		_blanks(nblanks);
	}
	_insmode(0);
}

/*
 * NAME:        _ins_string
 *
 * FUNCTION:
 *
 *      Insert the given string on the screen.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      This is like _showstring but you know you're in insert mode.
 */

static
_ins_string(sline, scol, first, last)
int sline, scol;
chtype *first, *last; 
{
	int len = last-first+1;

#ifdef DEBUG
	if (outf) fprintf(outf, "_ins_string at (%d, %d) %d chars\n",
		sline, scol, len);
#endif
	_pos(sline, scol);
	if( len > 1 && parm_ich )
	{
		/* Insert the characters and then draw on the blanks */
		_inschars(len);
		_showstring(sline, scol, first, last, NULL);
	}
	else
	{
		/*
		 * Type the characters in "insert mode".  This includes
		 * having to send insert_character before each character
		 * is output.
		 */
		_insmode(1);
		_showstring(sline, scol, first, last, NULL);
	}
	_insmode(0);
}
