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
static char *rcsid = "@(#)$RCSfile: xtob.c,v $ $Revision: 1.1.3.2 $ (DEC) $Date: 1992/04/14 13:48:07 $";
#endif

#include <sys/param.h>

/*
 * Convert an ASCII string of hex characters to a binary number.
 */

xtob(str)
	char *str;
{
	register int hexnum;

	if (str == NULL || *str == NULL)
		return(0);
	hexnum = 0;
	if (str[0] == '0' && str[1] == 'x')
		str = str + 2;
	for ( ; *str; str++) {
		if (*str >= '0' && *str <= '9')
			hexnum = hexnum * 16 + (*str - 48);
		else if (*str >= 'a' && *str <= 'f')
			hexnum = hexnum * 16 + (*str - 87);
		else if (*str >= 'A' && *str <= 'F')
			hexnum = hexnum * 16 + (*str - 55);
	}
	return(hexnum);
}

