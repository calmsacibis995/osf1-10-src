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
static char     *sccsid = "@(#)$RCSfile: atouint.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:36:47 $";
#endif
/*
 */

/*
 * atouint - convert an ascii string to an unsigned long, with error checking
 */
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>

int
atouint(str, uval)
	char *str;
	u_int *uval;
{
	register u_int u;
	register char *cp;

	cp = str;
	if (*cp == '\0')
		return 0;

	u = 0;
	while (*cp != '\0') {
		if (!isdigit(*cp))
			return 0;
		if (u > 429496729 || (u == 429496729 && *cp >= '6'))
			return 0;	/* overflow */
		u = (u << 3) + (u << 1);
		u += *cp++ - '0';	/* ascii dependent */
	}

	*uval = u;
	return 1;
}
