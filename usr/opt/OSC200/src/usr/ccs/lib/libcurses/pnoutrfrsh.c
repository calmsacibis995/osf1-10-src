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
static char rcsid[] = "@(#)$RCSfile: pnoutrfrsh.c,v $ $Revision: 4.2.6.3 $ (DEC) $Date: 1993/09/02 17:54:16 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "pnoutrfrsh.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:34:29"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   pnoutrefresh
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
 * make the current screen look like "win" over the area covered by
 * win.
 *
 */

#include	"cursesext.h"

extern	WINDOW *lwin;

/*
 * NAME:        pnoutrefresh
 *
 * FUNCTION:
 *
 *      Put out pad but don't actually update screen.
 */

pnoutrefresh(pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol)
register WINDOW	*pad;
int pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol;
{
	register int pr, r, c;
	register chtype	*nsp, *lch;

# ifdef DEBUG
	if(outf) fprintf(outf,
		"PREFRESH(pad %x, pcorner %d,%d, smin %d,%d, smax %d,%d)",
		pad, pminrow, pmincol, sminrow, smincol, smaxrow, smaxcol);
	_dumpwin(pad);
	if(outf) fprintf(outf, "PREFRESH:\n\tfirstch\tlastch\n");
# endif

	/* Make sure everything fits */
	if (pminrow < 0) pminrow = 0;
	   else if (pminrow >= pad->_maxy) pminrow = pad->_maxy-1;
	if (pmincol < 0) pmincol = 0;
	   else if (pmincol >= pad->_maxx) pmincol = pad->_maxx-1;
	if (sminrow < 0) sminrow = 0;
	   else if (sminrow > smaxrow) sminrow = smaxrow;
	if (smincol < 0) smincol = 0;
	   else if (smincol > smaxcol) smincol = smaxcol;
	if (smaxrow >= lines) smaxrow = lines-1;
	   else if (smaxrow < 0) smaxrow = 0;
	if (smaxcol >= columns) smaxcol = columns-1;
	    else if (smaxcol < 0) smaxcol = 0;
	if (smaxrow - sminrow > pad->_maxy - pminrow - 1)
		smaxrow = sminrow + (pad->_maxy - pminrow - 1);
	if (smaxcol - smincol > pad->_maxx - pmincol - 1)
	        smaxcol = smincol + (pad->_maxx - pmincol - 1);

	/* Copy it out, like a refresh, but appropriately offset */
	for (pr=pminrow,r=sminrow; r <= smaxrow; r++,pr++) {
		/* No record of what previous loc looked like, so do it all */
		lch = &pad->_y[pr][pad->_maxx-1];
		nsp = &pad->_y[pr][pmincol];
		_ll_move(r, smincol);
		for (c=smincol; nsp<=lch; c++) {
			if (SP->virt_x++ < columns && c <= smaxcol)
				*SP->curptr++ = *nsp++;
			else
				break;
		}
		pad->_firstch[pr] = _NOCHANGE;
	}

	/*
         * The pad's current (y,x) coordinates, namely (pad->_cury,pad->_curx),
         * are the last updated coordinates on the pad.  These coordinates
         * may not correspond to the part of the pad which pnoutrefresh is
         * mapping to the screen.  Therefore, we only have the cursor
         * track (via "_ll_move") on the screen, if the pad's current (y,x)
         * coordinates do, in fact, correspond to the part of the pad that
         * pnoutrefresh is mapping.  Otherwise, we leave the cursor where
         * it is.  (This fixes QAR# 13671.)
         */

	if ( (pad->_cury >= pminrow) &&
	    (pad->_cury <= pminrow + (smaxrow-sminrow+1)) &&
	    (pad->_curx >= pmincol) &&
	    (pad->_curx <= pmincol + (smaxcol-smincol+1)) )

	            _ll_move(sminrow + pad->_cury - pminrow,
			     smincol + pad->_curx - pmincol);    /* 001 */
	lwin = pad;
	return OK;
}
