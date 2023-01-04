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
 * HISTORY
 */
#ifndef lint
static char *rcsid = "@(#)$RCSfile: sia_getmsg.c,v $ $Revision: 1.1.2.3 $ (DEC) $Date: 1993/06/08 02:06:55 $";
#endif
/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#define sia_getamsg __sia_getamsg
#pragma weak sia_getamsg = __sia_getamsg
#endif

#ifdef MSG
#include <nl_types.h>

/* sia_getamsg -- combined catopen/catgets interface to avoid yet more
 * global variables in the library.
 *
 * Called with the catalog file name for catopen, and the set,msg,defstr of
 * catgets.
 *
 * Returns the fetched message if it's in the catalog (and the catalog can
 * be found), else returns defstr.
 *
 * The catalogs this routine opens are not closed until program exit or
 * an exec.  Caveat emptor.
 */

char *
sia_getamsg(const char *path, int set, int msgnum, const char *defstr)
{
	nl_catd catd;

	catd = catopen(path, 0);
	return catgets(catd, set, msgnum, defstr);
}

#else /* def MSG */

/* ARGSUSED */

char *
sia_getamsg(const char *path, int set, int msgnum, const char *defstr)
{
	return defstr;
}

#endif /* ndef MSG */
