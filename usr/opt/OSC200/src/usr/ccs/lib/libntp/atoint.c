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
static char     *sccsid = "@(#)$RCSfile: atoint.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1992/05/04 17:36:34 $";
#endif
/*
 */

/*
 * atoint - convert an ascii string to a signed long, with error checking
 */
#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <sys/types.h>

int
atoint(str, ival)
	char *str;
	int *ival;
{
	register u_int u;
	register char *cp;
	register int isneg;
	register int oflow_digit;

	cp = str;

	if (*cp == '-') {
		cp++;
		isneg = 1;
		oflow_digit = '8';
	} else {
		isneg = 0;
		oflow_digit = '7';
	}

	if (*cp == '\0')
		return 0;

	u = 0;
	while (*cp != '\0') {
		if (!isdigit(*cp))
			return 0;
		if (u > 214748364 || (u == 214748364 && *cp > oflow_digit))
			return 0;	/* overflow */
		u = (u << 3) + (u << 1);
		u += *cp++ - '0';	/* ascii dependent */
	}

	if (isneg)
		*ival = -((int)u);
	else 
		*ival = (int)u;
	return 1;
}
