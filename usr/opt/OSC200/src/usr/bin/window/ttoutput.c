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
static char	*sccsid = "@(#)$RCSfile: ttoutput.c,v $ $Revision: 4.2 $ (DEC) $Date: 1991/09/20 01:12:11 $";
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
 * Copyright (c) 1983 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by the University of California, Berkeley.  The name of the
 * University may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 * 	ttinit.c	3.20 (Berkeley) 6/29/88
 */


#include "ww.h"
#include "tt.h"
#include <sys/errno.h>

/*
 * Buffered output package.
 * We need this because stdio fails on non-blocking writes.
 */

ttflush()
{
	register char *p;
	register n;
	extern errno;

	wwnflush++;
	for (p = tt_ob; p < tt_obp;) {
		wwnwr++;
		n = write(1, p, tt_obp - p);
		if (n < 0) {
			wwnwre++;
			if (errno != EWOULDBLOCK) {
				/* can't deal with this */
				p = tt_obp;
			}
		} else if (n == 0) {
			/* what to do? */
			wwnwrz++;
		} else {
			wwnwrc += n;
			p += n;
		}
	}
	tt_obp = tt_ob;
}

ttputs(s)
register char *s;
{
	while (*s)
		ttputc(*s++);
}

ttwrite(s, n)
	register char *s;
	register n;
{
	switch (n) {
	case 0:
		break;
	case 1:
		ttputc(*s);
		break;
	case 2:
		if (tt_obe - tt_obp < 2)
			ttflush();
		*tt_obp++ = *s++;
		*tt_obp++ = *s;
		break;
	case 3:
		if (tt_obe - tt_obp < 3)
			ttflush();
		*tt_obp++ = *s++;
		*tt_obp++ = *s++;
		*tt_obp++ = *s;
		break;
	case 4:
		if (tt_obe - tt_obp < 4)
			ttflush();
		*tt_obp++ = *s++;
		*tt_obp++ = *s++;
		*tt_obp++ = *s++;
		*tt_obp++ = *s;
		break;
	case 5:
		if (tt_obe - tt_obp < 5)
			ttflush();
		*tt_obp++ = *s++;
		*tt_obp++ = *s++;
		*tt_obp++ = *s++;
		*tt_obp++ = *s++;
		*tt_obp++ = *s;
		break;
	default:
		while (n > 0) {
			register m;

			while ((m = tt_obe - tt_obp) == 0)
				ttflush();
			if ((m = tt_obe - tt_obp) > n)
				m = n;
			bcopy(s, tt_obp, m);
			tt_obp += m;
			s += m;
			n -= m;
		}
	}
}
