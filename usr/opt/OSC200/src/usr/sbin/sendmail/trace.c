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
static char	*sccsid = "@(#)$RCSfile: trace.c,v $ $Revision: 4.2.2.2 $ (DEC) $Date: 1993/09/07 19:09:06 $";
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
#if !defined(lint) && !defined(_NOIDENT)

#endif
/*
 * Copyright (c) 1983 Eric P. Allman
 * Copyright (c) 1988 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */
/*
#ifndef lint

#endif 
*/
# include "sendmail.h"

/*
**  TtSETUP -- set up for trace package.
**
**	Parameters:
**		vect -- pointer to trace vector.
**		size -- number of flags in trace vector.
**		defflags -- flags to set if no value given.
**
**	Returns:
**		none
**
**	Side Effects:
**		environment is set up.
*/

u_char		*tTvect;
int		tTsize;
static char	*DefFlags;

tTsetup(vect, size, defflags)
	u_char *vect;
	int size;
	char *defflags;
{
	tTvect = vect;
	tTsize = size;
	DefFlags = defflags;
}
/*
**  TtFLAG -- process an external trace flag description.
**
**	Parameters:
**		s -- the trace flag.
**
**	Returns:
**		none.
**
**	Side Effects:
**		sets/clears trace flags.
*/

tTflag(s)
	register char *s;
{
	int first, last;
	register int i;

	if (*s == '\0')
		s = DefFlags;

	for (;;)
	{
		/* find first flag to set */
		i = 0;
		while (isdigit(*s))
			i = i * 10 + (*s++ - '0');
		first = i;

		/* find last flag to set */
		if (*s == '-')
		{
			i = 0;
			while (isdigit(*++s))
				i = i * 10 + (*s - '0');
		}
		last = i;

		/* find the level to set it to */
		i = 1;
		if (*s == '.')
		{
			i = 0;
			while (isdigit(*++s))
				i = i * 10 + (*s - '0');
		}

		/* clean up args */
		if (first < 0)
		    first = 0;
		if (first >= tTsize)
			first = tTsize - 1;
		if (last >= tTsize)
			last = tTsize - 1;

		/* set the flags */
		while (first <= last)
			tTvect[first++] = i;

		/* more arguments? */
		if (*s++ == '\0')
			return;
	}
}
