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
static char rcsid[] = "@(#)$RCSfile: line_alloc.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:47:08 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/*** "line_alloc.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:24:08"; ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _line_alloc
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

/*
 * NAME:        _line_alloc
 *
 * FUNCTION:
 *
 *      Return a pointer to a new line structure.
 */

struct line *
_line_alloc ()
{
	register struct line   *rv = SP->freelist;
	char *calloc();

#ifdef DEBUG
	if(outf) fprintf(outf,
	"mem: _line_alloc (), prev SP->freelist %x\n", SP->freelist);
#endif
	if (rv) {
		SP->freelist = rv -> next;
	} else {
#ifdef NONSTANDARD
		_ec_quit("No lines available in line_alloc", "");
#else
		rv = (struct line *) calloc (1, sizeof *rv);
		rv -> body = (chtype *) calloc (columns, sizeof (chtype));
#ifdef PHASE2
		rv -> atr_body = (chtype *) calloc (columns, sizeof (chtype));
#endif
#endif
	}
	rv -> length = 0;
	rv -> hash = 0;
	return rv;
}
