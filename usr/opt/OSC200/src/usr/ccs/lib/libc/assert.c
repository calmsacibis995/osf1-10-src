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
static char	*sccsid = "@(#)$RCSfile: assert.c,v $ $Revision: 4.2.7.4 $ (DEC) $Date: 1993/09/23 18:29:28 $";
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
 * COMPONENT_NAME: (LIBCGEN) Standard C Library General Functions
 *
 * FUNCTIONS: __assert 
 *
 * ORIGINS: 3 27
 *
 * This module contains IBM CONFIDENTIAL code. -- (IBM
 * Confidential Restricted when combined with the aggregated
 * modules for this product)
 * OBJECT CODE ONLY SOURCE MATERIALS
 * (C) COPYRIGHT International Business Machines Corp. 1985, 1989 
 * All Rights Reserved
 *
 * US Government Users Restricted Rights - Use, duplication or
 * disclosure restricted by GSA ADP Schedule Contract with IBM Corp.
 *
 * Copyright (c) 1984 AT&T	
 * All Rights Reserved  
 *
 * THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	
 * The copyright notice above does not evidence any   
 * actual or intended publication of such source code.
 *
 * assert.c	1.10  com/lib/c/gen,3.1,9013 12/18/89 14:28:07
 */

/* name space pollution clean up */
#ifdef _NAME_SPACE_WEAK_STRONG
#include "pollution.h"
#endif
#ifdef MSG
#include "libc_msg.h"
#include <nl_types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
static char *locs(char *str, char *pat);
#endif

#ifdef KJI
#include <NLchar.h>
#endif

#define WRITE(s, n)	(void) write(2, (s), (n))
#define WRITESTR(s1, n, s2)	WRITE((s1), n), \
				WRITE((s2), (unsigned) strlen(s2))



/*
 * NAME:	__assert
 *
 * FUNCTION:	__assert - print out the "Assertion failed" message
 *		for the assert() macro (in assert.h)
 *
 * NOTES:	__assert prints out the
 *		Assertion failed: "expression", file "filename", line "linenum"
 *		message without using stdio.  It's called from the assert()
 *		macro in assert.h.
 *
 * RETURN VALUE DESCRIPTION:	none
 */

void
__assert(char *assertion, char *filename, int line_num)
{
	char linestr[14];
	char *p = &linestr[7];	/* p points to the first 'N' in linestr */
	char tmpbuf[16];	/* must be at least as large as linestr[] */
	int ddiv, digit;
#ifdef MSG
	char *msgbuf, *firstN;
	nl_catd catd = catopen(MF_LIBC,NL_CAT_LOCALE);
	int moveleft = 0;

        strcpy(linestr ,", line NNNNN\n");           
	msgbuf = NLcatgets(catd, MS_LIBC, M_ASSERT1, "Assertion failed: ");
	WRITESTR(msgbuf, (unsigned) strlen(msgbuf), assertion);

	msgbuf = NLcatgets(catd, MS_LIBC, M_ASSERT2, ", file ");
	WRITESTR(msgbuf, (unsigned) strlen(msgbuf), filename);

	/* Since the NLcatgets string is in RO storage, copy it elsewhere
	   so we can write the line number into it later. */
	strcpy(tmpbuf, NLcatgets(catd, MS_LIBC, M_ASSERT3, linestr));
	msgbuf = tmpbuf;
	if (*(p = locs(msgbuf, "NNNNN")) == 0) {
		msgbuf = linestr;
		p = locs(msgbuf, "NNNNN");
	}
	catclose(catd);

	firstN = p;

	/* convert line_num to ascii... */
	for (ddiv = 10000; ddiv != 0; line_num %= ddiv, ddiv /= 10)
		if ((digit = line_num/ddiv) != 0 || p != firstN || ddiv == 1)
			*p++ = digit + '0';
		else
			moveleft++;
	if (moveleft)
		do
			*p = *(p + moveleft);
		while (*p++);
	WRITE(msgbuf, (unsigned) strlen(msgbuf));
	(void) abort();
}

static char *
locs(char *str, char *pat) 
{
	char *s1, *s2; 
	int   more;

	if (str)
		while (*str) {
			for (s1 = pat, s2 = str, more = 0; *s1 && !more; ) 
				if (*s1++ != *s2++) 
					more++;
			if (!more)
				break;
#ifdef KJI
			str += NLchrlen (str);
#else
			++str;
#endif
		}
	return str;
}

#else	/* not MSG */
	WRITESTR("Assertion failed: ", 18, assertion);
	WRITESTR(", file ", 7, filename);

	/* convert line_num to ascii... */
	for (ddiv = 10000; ddiv != 0; line_num %= ddiv, ddiv /= 10)
		if ((digit = line_num/ddiv) != 0 || p != &linestr[7] || ddiv == 1)
			*p++ = digit + '0';
	*p++ = '\n';
	*p = '\0';
	WRITE(linestr, (unsigned) strlen(linestr));
	(void) abort();
}
#endif	/* not MSG */
