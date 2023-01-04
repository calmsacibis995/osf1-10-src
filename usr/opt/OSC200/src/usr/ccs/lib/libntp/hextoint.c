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
static char     *sccsid = "@(#)$RCSfile: hextoint.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:38:41 $";
#endif
/*
 */

/*
 * hextoint - convert an ascii string in hex to an unsigned
 *	      long, with error checking
 */
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>

int
hextoint(str, ival)
	char *str;
	u_int *ival;
{
	register u_int u;
	register char *cp;

	cp = str;

	if (*cp == '\0')
		return 0;

	u = 0;
	while (*cp != '\0') {
		if (!isxdigit(*cp))
			return 0;
		if (u >= 0x10000000)
			return 0;	/* overflow */
		u <<= 4;
		if (*cp <= '9')		/* very ascii dependent */
			u += *cp++ - '0';
		else if (*cp >= 'a')
			u += *cp++ - 'a' + 10;
		else
			u += *cp++ - 'A' + 10;
	}
	*ival = u;
	return 1;
}
