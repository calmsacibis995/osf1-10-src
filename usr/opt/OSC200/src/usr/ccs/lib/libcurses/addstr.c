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
static char rcsid[] = "@(#)$RCSfile: addstr.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 20:56:46 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "addstr.c  1.6  com/lib/curses,3.1,8943 10/16/89 23:03:19";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   waddstr
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
 *	Modification History
 *	--------------------
 * 001	Tom Woodburn		04 Jun 91
 *	Masked off sign-extended bits with MASK8 macro in call to
 *	waddch().
 */

# include	"cursesext.h"

/*
 * NAME:        waddstr
 *
 * FUNCTION:
 *
 *      This routine adds a string starting at (_cury,_curx)
 *
 */

#define MASK8 0xff	/* TJW 001 */

waddstr(win,str)
register WINDOW	*win; 
register char	*str;
{
# ifdef DEBUG
	if(outf)
	{
		if( win == stdscr )
		{
			fprintf(outf, "WADDSTR(stdscr, ");
		}
		else
		{
			fprintf(outf, "WADDSTR(%o, ", win);
		}
		fprintf(outf, "\"%s\")\n", str);
	}
# endif	/* DEBUG */
	while( *str )
	{
		if( waddch( win, ( chtype ) *str++ & MASK8 ) == ERR )
		{
			return ERR;
		}
	}
	return OK;
}
