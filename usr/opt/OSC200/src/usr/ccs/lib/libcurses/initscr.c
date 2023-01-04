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
static char rcsid[] = "@(#)$RCSfile: initscr.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:38:30 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "initscr.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:22:13"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   initscr
 *
 *
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1988
 * All Rights Reserved
 * Licensed Material - Property of IBM
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */

# include	"cursesext.h"
# include	<signal.h>

char	*calloc();
char	*malloc();
extern	char	*getenv();

extern	WINDOW	*makenew();

/*
 * NAME:        initscr
 *
 * FUNCTION:
 *
 *      This routine initializes the current and standard screen.
 */

WINDOW *
initscr()
{
	register char *sp;
	struct screen *scp;
	extern char *_c_why_not;
	
# ifdef DEBUG
	if (outf == NULL) {
		if( ( outf = fopen("trace", "w") ) == NULL)
		{
			perror("trace");
			exit(-1);
		}
	}
#endif

	if( ( sp = getenv( "TERM" ) ) == NULL )
	{
		sp = Def_term;
	}
# ifdef DEBUG
	if(outf) fprintf(outf, "INITSCR: term = %s\n", sp);
# endif
	if( ( scp = newterm( sp, stdout, stdin ) ) == NULL )
	{
		_ec_quit(_c_why_not, sp);
	}
	return stdscr;
}
