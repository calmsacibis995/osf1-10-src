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
static char rcsid[] = "@(#)$RCSfile: tgetnum.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 22:40:18 $";
#endif
/*
 * HISTORY
 */
/*** "tgetnum.c  1.7  com/lib/curses,3.1,9008 12/14/89 17:53:13"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   tgetnum
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


#include "cursesext.h"

#define	two( s1, s2 )	(s1 + 256 * s2 )
#define	twostr( str )	two( *str, str[ 1 ] )

/*
 * NAME:        tgetnum
 *
 * FUNCTION:
 *
 *      Simulation of termcap using terminfo.
 */

int
tgetnum(id)
char *id;
{
	int rv;

	switch (twostr(id)) {
	case two('c','o'): rv = columns; break;
	case two('i','t'): rv = init_tabs; break;
	case two('l','i'): rv = lines; break;
	case two('l','m'): rv = lines_of_memory; break;
	case two('s','g'): rv = magic_cookie_glitch; break;
	case two('p','b'): rv = padding_baud_rate; break;
	case two('v','t'): rv = virtual_terminal; break;
	case two('w','s'): rv = width_status_line; break;
	default: rv = -1;
	}
	return rv;
}
