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
static char rcsid[] = "@(#)$RCSfile: cntcostfn.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/06/12 21:08:45 $";
#endif
/*
 * HISTORY
 */
/*
 */ 
/***
 ***  "cntcostfn.c  1.5  com/lib/curses,3.1,8943 10/16/89 23:08:46";
 ***/
/*
 * COMPONENT_NAME: (LIBCURSE) Curses Library
 *
 * FUNCTIONS:   _countchar, _cost_fn
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

static counter = 0;

/*
 * NAME:        _countchar
 */

_countchar(ch)
char ch;
{
	counter++;
}

/*
 * NAME:        _cost_fn
 *
 * FUNCTION:
 *
 *      Figure out the _cost in characters to print this string.
 *
 * EXECUTION ENVIRONMENT:
 *
 *      Due to padding, we can't just use strlen, so instead we
 *      feed it through tputs and trap the results.
 *      Even if the terminal uses xon/xoff handshaking, count the
 *      pad chars here since they estimate the real time to do the
 *      operation, useful in calculating costs.
 */

_cost_fn(str, affcnt)
char *str;
{
	int save_xflag = xon_xoff;

	if (str == NULL)
		return INFINITY;
	counter = 0;
	xon_xoff = 0;
	tputs(str, affcnt, _countchar);
	xon_xoff = save_xflag;
	return counter;
}
