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
static char rcsid[] = "@(#)$RCSfile: doupdate.c,v $ $Revision: 4.2.6.2 $ (DEC) $Date: 1993/06/12 21:22:04 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "doupdate.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:14:35";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   doupdate
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

#include        "cursesext.h"
#include	<signal.h>

extern	void	_tstp();
extern  WINDOW *lwin;

/*
 * NAME:        doupdate
 *
 * FUNCTION:
 *
 *      Make the current screen look like "win" over the area covered by
 *      win.
 */

doupdate()
{
	int rc;
	extern int _endwin;
	int _outch();

#ifdef	DEBUG
	if(outf) fprintf( outf, "doupdate()\n" );
#endif	/* DEBUG */

	if( lwin == NULL )
	{
		return ERR;
	}

	if( _endwin )
	{
		/*
		 * We've called endwin since last refresh.  Undo the
		 * effects of this call.
		 */

		_fixdelay(FALSE, SP->fl_nodelay);
		if (stdscr->_use_meta)
			tputs(meta_on, 1, _outch);
		_endwin = FALSE;
		SP->doclear = TRUE;
		reset_prog_mode();
#ifdef	SIGTSTP
		signal(SIGTSTP, ((void (*)(int))_tstp));
#endif
	}

	/* Tell the back end where to leave the cursor */
	if( lwin->_leave )				/* 001 */
	{
#ifdef	DEBUG
		if(outf) fprintf( outf, "'_ll_move(-1, -1)' being done.\n" );
#endif	/* DEBUG */
		_ll_move(-1, -1);
	}
	else
	{
		if ( ! ( lwin->_flags&_ISPAD ) )		/* 001 */
		{					/* 001 */
#ifdef	DEBUG
if(outf) fprintf( outf,
"'lwin->_cury+lwin->_begy, lwin->_curx+lwin->_begx' being done.\n" );

#endif	/* DEBUG */
			_ll_move( lwin->_cury+lwin->_begy, lwin->_curx+lwin->_begx );
		}					/* 001 */
	}
#ifdef	DEBUG
if(outf) fprintf( outf, "doing 'rc = _ll_refresh(lwin->_use_idl)'.\n" );

#endif	/* DEBUG */
	rc = _ll_refresh(lwin->_use_idl);
#ifdef	DEBUG
	_dumpwin(lwin);
#endif	/* DEBUG */
	return rc;
}
