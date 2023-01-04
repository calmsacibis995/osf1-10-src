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
#if !defined(lint) && !defined(_NOIDENT)
static char rcsid[] = "@(#)$RCSfile: subdir.c,v $ $Revision: 4.3.5.2 $ (DEC) $Date: 1993/09/07 16:07:58 $";
#endif
/*
 * Copyright International Business Machines Corp. 1989
 * Unpublished Work
 * All Rights Reserved
 * Licensed Material - Property of IBM
 */
/*
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 */ 
/*
subdir.c	1.1  com/cmd/uucp,3.1,9013 9/27/89 15:20:59";
*/

#include "uucp.h"

/*LINTLIBRARY*/

/*
 * By Tom Truscott, March 1983
 *
 * Prefix table.
 * If a prefix is "abc", for example,
 * then any file Spool/abc... is mapped to Spool/abc/abc... .
 * The first prefix found is used, so D.foo should preceed D. in table.
 *
 * Each prefix must be a subdirectory of Spool, owned by uucp!
 * Remember: use cron to uuclean these directories daily,
 * and check them manually every now and then.  Beware complacency!
 */

#define	DLocalX		"X."
#define	DLocal		"D."

static char *dprefix[] = {
	DLocalX,	/* Outbound 'xqt' request files */
	DLocal,		/* Outbound data files */
	"D.",		/* Other "D." files (remember the "."!) */
	"C.",		/* "C." subdirectory */
	"X.",		/* "X." subdirectory */
	"TM.",		/* Temporaries for inbound files */
	0
};


/*
 * filename mapping kludges to put uucp work files in other directories.
 */

#define	BUFLEN	50

static	char fn1[BUFLEN], fn2[BUFLEN];	/* remapped filename areas */
static	int	inspool;		/* true iff working dir is Spool */

/*
 * return (possibly) remapped string s
 */
char *
subfile(as)
char *as;
{
	register char *s, **p;
	register int n;
	static char *tptr = NULL;

	/* Alternate buffers so "link(subfile(a), subfile(b))" works */
	if (tptr != fn1)
		tptr = fn1;
	else
		tptr = fn2;

	s = as;
	tptr[0] = '\0';

	/* if s begins with Spool/, copy that to tptr and advance s */
	if (strncmp(s, Spool, n = strlen(Spool)) == 0 && s[n] == '/') {
		if (!inspool) {
			strcpy(tptr, Spool);
			strcat(tptr, "/");
		}
		s += n + 1;
	}
	else
		if (!inspool)
			return as;

	/* look for first prefix which matches, and make subdirectory */
	for (p = &dprefix[0]; *p; p++) {
		if (strncmp(s, *p, n = strlen(*p))==0 && s[n] && s[n] != '/') {
			strcat(tptr, *p);
			strcat(tptr, "/");
			strcat(tptr, s);
			return tptr;
		}
	}
	return as;
}

/*
 * save away filename
 */
subchdir(s)
register char *s;
{
	inspool = (strcmp(s, Spool) == 0);
	return chdir(s);
}
