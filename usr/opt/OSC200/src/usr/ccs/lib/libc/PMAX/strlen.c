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
static char	*sccsid = "@(#)$RCSfile: strlen.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 03:09:57 $";
#endif 
/*
 */
/*
 * (c) Copyright 1990, OPEN SOFTWARE FOUNDATION, INC.
 * ALL RIGHTS RESERVED
 */
/*
 * OSF/1 Release 1.0
 */
/*
 * strlen.c
 *
 *	Revision History:
 *
 * 12-Mar-91	Fred Canter
 *	MIPS C 2.20+
 *
 */
#ifndef lint

#endif not lint
/* ------------------------------------------------------------------ */
/* | Copyright Unpublished, MIPS Computer Systems, Inc.  All Rights | */
/* | Reserved.  This software contains proprietary and confidential | */
/* | information of MIPS and its suppliers.  Use, disclosure or     | */
/* | reproduction is prohibited without the prior express written   | */
/* | consent of MIPS.                                               | */
/* ------------------------------------------------------------------ */

/*
 * Returns the number of non-NULL bytes in string argument.
 * Original code:
 *	strlen(s)
 *	register char *s;
 *	{
 *		register n;
 *		n = 0;
 *		while (*s++)
 *			n++;
 *		return(n);
 *	}
 */

/* Code optimized for mips.  4 cycles/byte. */

#include <string.h>

#ifdef	_NONSTD_TYPES
int
#else
size_t
#endif
strlen(const char *s)
{
	register char *p = (char *) s + 1;
	register unsigned c;	/* c exists only because UOPT wastes a
				   register (but no cycles) if the
				   while is written as *s++ != '\0' */
	do {
		c = *s++;
	} while (c != '\0');
	return s - p;
}
